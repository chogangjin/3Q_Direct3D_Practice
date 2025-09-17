#pragma once
#include <chrono>

class TimeSystem
{
	static std::chrono::high_resolution_clock::time_point prevTime;
	static std::chrono::high_resolution_clock::time_point currTime;
	static TimeSystem* m_pInstance;
public:
	std::chrono::duration<float> elapsed;
	float deltaTime = 0;
	TimeSystem() { m_pInstance = this; }
	
	void Initialize();
	void Update();
	static TimeSystem* GetInstance();
};

