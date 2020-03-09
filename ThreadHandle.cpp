#include "ThreadHandle.h"
#include <process.h>
ThreadHandle::ThreadHandle() : 
m_beginThread(NULL),
m_EndThread(NULL),
m_thread(NULL)
{
	InitialThread();
}
ThreadHandle::~ThreadHandle()
{
	DestroyThread();
}
unsigned __stdcall ThreadHandle::ThreadRunning(LPVOID lpParameter)
{
	// �������test*�^�ɃL���X�g���āA�{���̏��������s����B
	reinterpret_cast<ThreadHandle*>(lpParameter)->ThreadExcecute();

	return 0;
}
HRESULT ThreadHandle::InitialThread()
{
	m_thread = (HANDLE) _beginthreadex(
		NULL,
		0,
		&ThreadHandle::ThreadRunning,
		this,
		0,
		NULL);

	m_beginThread = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_EndThread = CreateEvent(NULL, FALSE, FALSE, NULL);

	return S_OK;
}


void ThreadHandle::DestroyThread()
{
	if (m_thread)
	{
		CloseHandle(m_thread);
	}
	if (m_beginThread)
	{
		CloseHandle(m_beginThread);
	}
	if (m_EndThread)
	{
		CloseHandle(m_EndThread);
	}
}