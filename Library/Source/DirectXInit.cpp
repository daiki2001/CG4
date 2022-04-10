#include "./Header/DirectXInit.h"
#include <string>

DirectXInit::DirectXInit() :
	Win32API(),
	clearColor{ 0, 0, 0, 0 },
	dev{},
	dxgiFactory{},
	swapchain{},
	cmdAllocator{},
	cmdList{},
	cmdQueue{},
	rtvHeaps{},
	adapters{},
	tmpAdapter{},
	levels{ D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0,
	D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 },
	featureLevel{},
	cmdQueueDesc{},
	swapchainDesc{},
	heapDesc{},
	backBuffer(2),
	fence{},
	fenceVal(0),
	depthResDesc{},
	depthBuffer{},
	dsvHeapDesc{},
	dsvHeap{},
	dsvDesc{},
	bbIndex{}
{
}

void DirectXInit::DebugLayer()
{
	ID3D12Debug* debugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
	{
		debugController->EnableDebugLayer();
	}
}

DirectXInit* DirectXInit::GetInstance()
{
	// インスタンスの生成
	static DirectXInit instance;

	return &instance;
}

HRESULT DirectXInit::Init()
{
	HRESULT hr;

	WindowInit();
	DebugLayer();

	// DXGIファクトリーの生成
	hr = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));
	if (FAILED(hr))
	{
		return hr;
	}

	for (int i = 0; dxgiFactory->EnumAdapters1(i, &tmpAdapter) != DXGI_ERROR_NOT_FOUND; i++)
	{
		adapters.push_back(tmpAdapter); //動的配列に追加する
	}

	for (size_t i = 0; i < adapters.size(); i++)
	{
		DXGI_ADAPTER_DESC1 adesc;
		adapters[i]->GetDesc1(&adesc); //アダプターの情報を取得

		// ソフトウェアデバイスを回避
		if (adesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			continue;
		}

		std::wstring strDesc = adesc.Description; //アダプター名
		// Intel UHD Graphics (オンボードグラフィック)を回避
		if (strDesc.find(L"Intel") == std::wstring::npos)
		{
			tmpAdapter = adapters[i]; //採用
			break;
		}
	}

	for (int i = 0; i < _countof(levels); i++)
	{
		// 採用したアダプターでデバイスを生成
		hr = D3D12CreateDevice(tmpAdapter.Get(), levels[i], IID_PPV_ARGS(&dev));
		if (hr == S_OK)
		{
			// デバイスを生成出来た時点でループを抜ける
			featureLevel = levels[i];
			break;
		}
	}
	if (FAILED(hr))
	{
		// デバイスが無かった場合リターン
		return hr;
	}

	// コマンドアロケータを生成
	hr = dev->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&cmdAllocator)
	);
	if (FAILED(hr))
	{
		return hr;
	}

	// コマンドリストを生成
	hr = dev->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		cmdAllocator.Get(),
		nullptr,
		IID_PPV_ARGS(&cmdList)
	);
	if (FAILED(hr))
	{
		return hr;
	}

	hr = dev->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&cmdQueue));
	if (FAILED(hr))
	{
		return hr;
	}

	swapchainDesc.Width = windowWidth;
	swapchainDesc.Height = windowHeight;
	swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; //色情報の書式
	swapchainDesc.SampleDesc.Count = 1; //マルチサンプルしない
	swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER; //バックバッファ用
	swapchainDesc.BufferCount = 2; //バッファ数を2つに設定
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; //フリップ後は破棄
	swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	ComPtr<IDXGISwapChain1> swapchain1;
	hr = dxgiFactory->CreateSwapChainForHwnd(
		cmdQueue.Get(),
		Win32API::GetHwnd(),
		&swapchainDesc,
		nullptr,
		nullptr,
		&swapchain1
	);
	if (FAILED(hr))
	{
		return hr;
	}
	hr = swapchain1.As(&swapchain);
	if (FAILED(hr))
	{
		return hr;
	}

	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; //レンダーターゲットビュー
	heapDesc.NumDescriptors = 2; //裏表の2つ
	hr = dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&rtvHeaps));
	if (FAILED(hr))
	{
		return hr;
	}

	for (UINT i = 0; i < 2; i++)
	{
		// スワップチェインからバッファを取得
		hr = swapchain->GetBuffer(i, IID_PPV_ARGS(&backBuffer[i]));
		if (FAILED(hr))
		{
			return hr;
		}
		// スデスクリプタヒープのハンドルを取得
		CD3DX12_CPU_DESCRIPTOR_HANDLE handle =
			CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvHeaps->GetCPUDescriptorHandleForHeapStart(),
				i,
				dev->GetDescriptorHandleIncrementSize(heapDesc.Type)
			);
		// レンダーターゲットビューの生成
		dev->CreateRenderTargetView(backBuffer[i].Get(), nullptr, handle);
	}

	CreateDepthBuffer();

	hr = dev->CreateFence(fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));

	return hr;
}

void DirectXInit::ClearScreen()
{
#pragma region ChangeResourceBarrier

	// バックバッファの番号を取得(2つなので0番か1番)
	bbIndex = swapchain->GetCurrentBackBufferIndex();

	cmdList->ResourceBarrier(
		1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			backBuffer[bbIndex].Get(),
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET
		)
	);

#pragma endregion

#pragma region DrawingDestination

	// 描画先の指定
	// レンダーターゲットビュー用ディスクリプタヒープのハンドルを取得
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvH =
		CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvHeaps->GetCPUDescriptorHandleForHeapStart(),
			(INT)bbIndex,
			dev->GetDescriptorHandleIncrementSize(heapDesc.Type)
		);
	// 深度ステンシルビュー用ディスクリプタヒープのハンドルを取得
	D3D12_CPU_DESCRIPTOR_HANDLE dsvH = dsvHeap->GetCPUDescriptorHandleForHeapStart();
	cmdList->OMSetRenderTargets(1, &rtvH, false, &dsvH);

#pragma endregion

#pragma region ClearScreen

	// 画面クリア
	cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);
	cmdList->ClearDepthStencilView(dsvH, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

#pragma endregion
}

void DirectXInit::ScreenFlip()
{
#pragma region RetrunResourceBarrier

	// リソースバリアを戻す
	cmdList->ResourceBarrier(
		1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			backBuffer[bbIndex].Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT
		)
	);

#pragma endregion

#pragma region FlashCommand

	// 命令のクローズ
	cmdList->Close();
	// コマンドリストの実行
	ID3D12CommandList* cmdLists[] = { cmdList.Get() }; //コマンドリストの配列
	cmdQueue->ExecuteCommandLists(1, cmdLists);

	// バッファをフリップ(裏表の入れ替え)
	swapchain->Present(1, 0);

	// コマンドキューの実行完了を待つ
	cmdQueue->Signal(fence.Get(), ++fenceVal);
	if (fence->GetCompletedValue() != fenceVal)
	{
		HANDLE event = CreateEvent(nullptr, false, false, nullptr);
		fence->SetEventOnCompletion(fenceVal, event);
		WaitForSingleObject(event, INFINITE);
		CloseHandle(event);
	}

#pragma endregion

#pragma region ScreenFlip

	cmdAllocator->Reset(); //キューをクリア
	cmdList->Reset(cmdAllocator.Get(), nullptr); //再びコマンドリストを貯める準備

#pragma endregion
}

HRESULT DirectXInit::CreateDepthBuffer()
{
	HRESULT hr;

	// リソース設定
	depthResDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		DXGI_FORMAT_D32_FLOAT,
		windowWidth,
		windowHeight,
		1,
		0,
		1,
		0,
		D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
	);

	// リソース生成
	hr = dev->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&depthResDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE, //深度値書き込みに使用
		&CD3DX12_CLEAR_VALUE(DXGI_FORMAT_D32_FLOAT, 1.0f, 0),
		IID_PPV_ARGS(&depthBuffer)
	);
	if (FAILED(hr))
	{
		return hr;
	}

	// 深度ビュー用デスクリプタヒープ作成
	dsvHeapDesc.NumDescriptors = 1; //深度ビューは1つ
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV; //デプスステンシルビュー
	hr = dev->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsvHeap));
	if (FAILED(hr))
	{
		return hr;
	}

	// 深度ビュー作成
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT; //深度値フォーマット
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dev->CreateDepthStencilView(
		depthBuffer.Get(),
		&dsvDesc,
		dsvHeap->GetCPUDescriptorHandleForHeapStart()
	);

	return hr;
}

int DirectXInit::SetWindowSize(int width, int height)
{
	if (width <= 0 || height <= 0)
	{
		return -1;
	}

	windowWidth = width;
	windowHeight = height;

	return 0;
}
