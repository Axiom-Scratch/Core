#include "Application.h"
#include <chrono>

void Application::Run()
{
    OnInit();

    using clock = std::chrono::high_resolution_clock;
    auto last = clock::now();

    while (m_Running)
    {
        auto now = clock::now();
        float dt = std::chrono::duration<float>(now - last).count();
        last = now;

        OnUpdate(dt);
        OnRender();
    }

    OnShutdown();
}

void Application::Close()
{
    m_Running = false;
}