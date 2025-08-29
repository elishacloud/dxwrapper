#pragma once

class ShadowSurfaceStorage
{
public:
    ShadowSurfaceStorage() = default;
    ~ShadowSurfaceStorage() {}

    // Create N shadow surfaces (all nullptr initially)
    void Initialize(size_t count)
    {
        ReleaseAll();
        surfaces.resize(count, nullptr);
        currentIndex = 0;
    }

    // Assign a surface pointer into the container (AddRef for safety)
    void SetSurface(size_t index, m_IDirect3DSurface9* surf)
    {
        if (index >= surfaces.size()) return;

        if (surfaces[index]) reinterpret_cast<IDirect3DSurface9*>(surfaces[index])->Release();
        surfaces[index] = surf;
    }

    m_IDirect3DSurface9* ShadowSurfaceStorage::GetSurface(size_t iBackBuffer) const
    {
        if (surfaces.empty() || iBackBuffer >= surfaces.size())
            return nullptr;

        // Calculate rotated index
        size_t idx = (currentIndex + iBackBuffer) % surfaces.size();
        return surfaces[idx];
    }

    // Return how many shadow surfaces exist
    size_t Count() const { return surfaces.size(); }

    // Return the current "shadow back buffer"
    m_IDirect3DSurface9* GetCurrentBackBuffer() const
    {
        if (surfaces.empty()) return nullptr;
        return surfaces[currentIndex];
    }

    // Return the "front buffer" (the one that would be presented)
    m_IDirect3DSurface9* GetCurrentFrontBuffer() const
    {
        if (surfaces.empty()) return nullptr;

        // For N>1, front = the one just before current
        size_t idx = (currentIndex + surfaces.size() - 1) % surfaces.size();
        return surfaces[idx];
    }

    // Advance to next back buffer (call this each Present)
    void Rotate()
    {
        if (!surfaces.empty())
        {
            currentIndex = (currentIndex + 1) % surfaces.size();
        }
    }

    // Check if a given pointer is one of our shadow surfaces
    bool IsShadowSurface(const m_IDirect3DSurface9* ptr) const
    {
        for (auto* s : surfaces)
        {
            if (s == ptr) return true;
        }
        return false;
    }

    // Release everything
    void ReleaseAll()
    {
        for (auto*& s : surfaces)
        {
            if (s) { reinterpret_cast<IDirect3DSurface9*>(s)->Release(); s = nullptr; }
        }
        surfaces.clear();
        currentIndex = 0;
    }

private:
    std::vector<m_IDirect3DSurface9*> surfaces;
    size_t currentIndex = 0;
};
