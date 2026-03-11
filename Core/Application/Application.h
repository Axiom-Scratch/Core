#pragma once

class Application
{
public:
    void Run();
    void Close();

protected:
    virtual void OnInit() {}
    virtual void OnUpdate(float dt) {}
    virtual void OnRender() {}
    virtual void OnShutdown() {}

private:
    bool m_Running = true;
};