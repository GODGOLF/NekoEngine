#ifndef D3D12_CLASS_H
#define D3D12_CLASS_H
#include "DXInF.h"

class D3D12Class : public DXInF {
public:
	D3D12Class();
	~D3D12Class();
	HRESULT OnInit(HWND hwnd, UINT width, UINT height) override;
	void BindMainRenderTarget() override;
	void EndDraw() override;
	void OnDestroy() override;
	XMVECTORF32 GetDefaultColorBg();
	void Reset();

	void* operator new(size_t i)
	{
		return _mm_malloc(i, 16);
	}
	void operator delete(void* p)
	{
		_mm_free(p);
	}
private:

	static const uint8_t m_NumFrames = 3;
	Microsoft::WRL::ComPtr<ID3D12Device2> m_Device;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue;
	Microsoft::WRL::ComPtr<IDXGISwapChain4> m_SwapChain;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_BackBuffers[m_NumFrames];
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_CommandList;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_CommandAllocators[m_NumFrames];
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_RTVDescriptorHeap;
	// Synchronization objects
	Microsoft::WRL::ComPtr<ID3D12Fence> g_Fence;
	uint64_t g_FenceValue = 0;
	uint64_t g_FrameFenceValues[m_NumFrames] = {};
	HANDLE g_FenceEvent;
private:
	void EnableDebugLayer();
};
#endif // !D3D11_CLASS_H