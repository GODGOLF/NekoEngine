#ifndef _THREAD_HANDLE_H_
#define _THREAD_HANDLE_H_
#include <Windows.h>

class ThreadHandle
{
public:
	ThreadHandle();
	virtual ~ThreadHandle();
	void BindEndEventHandle();
	void BindBeginEventHandle();
	HANDLE GetBeginThreadHandle();
	HANDLE GetEndThreadHandle();
private:
	HANDLE m_thread;
	static unsigned __stdcall ThreadRunning(LPVOID lpParameter);
	HRESULT InitialThread();
	//when don't use thread anymore, call this function
	void DestroyThread();
	virtual void ThreadExcecute() {};
	
protected:
	HANDLE m_beginThread;
	HANDLE m_endThread;
	
};

#endif // !_THREAD_HANDLE_H_



