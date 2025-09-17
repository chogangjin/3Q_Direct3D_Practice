#include "framework.h"
#include "TimeSystem.h"

std::chrono::high_resolution_clock::time_point TimeSystem::prevTime = std::chrono::high_resolution_clock::now();
std::chrono::high_resolution_clock::time_point TimeSystem::currTime = std::chrono::high_resolution_clock::now();

TimeSystem* TimeSystem::m_pInstance = nullptr;

void TimeSystem::Initialize()
{
}

void TimeSystem::Update()
{
	prevTime = currTime;
	currTime = std::chrono::high_resolution_clock::now();
	elapsed = currTime - prevTime;
	deltaTime = elapsed.count();
}

TimeSystem* TimeSystem::GetInstance()
{
	if(nullptr == m_pInstance)
	{
		m_pInstance = new TimeSystem;
	}
	
	return m_pInstance;
}
