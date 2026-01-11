#pragma once

#include "Types.h"
#include <deque>
#include <mutex>
#include <functional>

namespace Ephemery {

class ImageStorage {
public:
    ImageStorage();
    ~ImageStorage();

    bool Initialize(const std::wstring& storagePath, uint32_t maxImages);
    void Shutdown();

    bool AddImage(const std::wstring& filePath, CaptureSource source);
    std::vector<CapturedImage> GetAllImages() const;
    std::vector<std::wstring> GetAllPaths() const;
    void Clear();           // リストとファイルの両方を削除
    void ClearList();       // リストのみクリア（ファイルは残す）
    size_t Count() const;

    void SetMaxImages(uint32_t maxImages);
    uint32_t GetMaxImages() const;

    std::wstring GenerateFilePath() const;
    std::wstring GetStoragePath() const;

    void LoadExistingImages();

private:
    void EnforceMaxLimit();
    bool DeleteOldestImage();
    bool EnsureDirectoryExists();
    std::wstring GenerateTimestamp() const;

    std::wstring m_storagePath;
    uint32_t m_maxImages = 10;
    uint32_t m_nextSequence = 1;
    std::deque<CapturedImage> m_images;
    mutable std::mutex m_mutex;
};

} // namespace Ephemery
