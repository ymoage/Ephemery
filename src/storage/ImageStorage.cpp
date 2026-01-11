#include "ImageStorage.h"
#include "util/Logger.h"
#include <Windows.h>
#include <ShlObj.h>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <algorithm>

namespace Ephemery {

ImageStorage::ImageStorage() = default;

ImageStorage::~ImageStorage() {
    Shutdown();
}

bool ImageStorage::Initialize(const std::wstring& storagePath, uint32_t maxImages) {
    std::lock_guard<std::mutex> lock(m_mutex);

    m_storagePath = storagePath;
    m_maxImages = maxImages;

    if (!EnsureDirectoryExists()) {
        LOG_ERROR(L"Failed to create storage directory: " + m_storagePath);
        return false;
    }

    LOG_INFO(L"ImageStorage initialized: " + m_storagePath);
    return true;
}

void ImageStorage::Shutdown() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_images.clear();
}

bool ImageStorage::AddImage(const std::wstring& filePath, CaptureSource source) {
    std::lock_guard<std::mutex> lock(m_mutex);

    // T035: ディスク容量チェック（10MB以上の空きが必要）
    ULARGE_INTEGER freeBytesAvailable;
    if (GetDiskFreeSpaceExW(m_storagePath.c_str(), &freeBytesAvailable, nullptr, nullptr)) {
        if (freeBytesAvailable.QuadPart < 10 * 1024 * 1024) {
            LOG_WARNING(L"Low disk space: " + std::to_wstring(freeBytesAvailable.QuadPart / (1024 * 1024)) + L"MB remaining");
            return false;
        }
    }

    CapturedImage image;
    image.filePath = filePath;
    image.capturedAt = std::chrono::system_clock::now();
    image.sequenceNumber = m_nextSequence++;
    image.source = source;

    m_images.push_back(image);

    EnforceMaxLimit();

    LOG_INFO(L"Added image: " + filePath);
    return true;
}

std::vector<CapturedImage> ImageStorage::GetAllImages() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return std::vector<CapturedImage>(m_images.begin(), m_images.end());
}

std::vector<std::wstring> ImageStorage::GetAllPaths() const {
    std::lock_guard<std::mutex> lock(m_mutex);

    std::vector<std::wstring> paths;
    paths.reserve(m_images.size());

    for (const auto& image : m_images) {
        paths.push_back(image.filePath);
    }

    return paths;
}

void ImageStorage::Clear() {
    std::lock_guard<std::mutex> lock(m_mutex);

    for (const auto& image : m_images) {
        DeleteFileW(image.filePath.c_str());
    }

    m_images.clear();
    LOG_INFO(L"Cleared all images (files deleted)");
}

void ImageStorage::ClearList() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_images.clear();
    LOG_INFO(L"Cleared image list (files preserved)");
}

size_t ImageStorage::Count() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_images.size();
}

void ImageStorage::SetMaxImages(uint32_t maxImages) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_maxImages = maxImages;
    EnforceMaxLimit();
}

uint32_t ImageStorage::GetMaxImages() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_maxImages;
}

std::wstring ImageStorage::GenerateFilePath() const {
    std::lock_guard<std::mutex> lock(m_mutex);

    std::wstring filename = L"ephemery_" + GenerateTimestamp() +
                           L"_" + std::to_wstring(m_nextSequence) + L".png";

    return m_storagePath + L"\\" + filename;
}

std::wstring ImageStorage::GetStoragePath() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_storagePath;
}

void ImageStorage::LoadExistingImages() {
    std::lock_guard<std::mutex> lock(m_mutex);

    WIN32_FIND_DATAW findData;
    std::wstring searchPath = m_storagePath + L"\\ephemery_*.png";

    HANDLE hFind = FindFirstFileW(searchPath.c_str(), &findData);
    if (hFind == INVALID_HANDLE_VALUE) {
        return;
    }

    std::vector<std::pair<FILETIME, std::wstring>> files;

    do {
        if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            std::wstring fullPath = m_storagePath + L"\\" + findData.cFileName;
            files.emplace_back(findData.ftCreationTime, fullPath);
        }
    } while (FindNextFileW(hFind, &findData));

    FindClose(hFind);

    std::sort(files.begin(), files.end(),
        [](const auto& a, const auto& b) {
            return CompareFileTime(&a.first, &b.first) < 0;
        });

    for (const auto& [ft, path] : files) {
        CapturedImage image;
        image.filePath = path;
        image.sequenceNumber = m_nextSequence++;
        image.source = CaptureSource::Screen;

        SYSTEMTIME st;
        FileTimeToSystemTime(&ft, &st);
        FILETIME localFt;
        FileTimeToLocalFileTime(&ft, &localFt);

        m_images.push_back(image);
    }

    EnforceMaxLimit();

    LOG_INFO(L"Loaded " + std::to_wstring(m_images.size()) + L" existing images");
}

void ImageStorage::EnforceMaxLimit() {
    while (m_images.size() > m_maxImages) {
        DeleteOldestImage();
    }
}

bool ImageStorage::DeleteOldestImage() {
    if (m_images.empty()) return false;

    const auto& oldest = m_images.front();
    if (DeleteFileW(oldest.filePath.c_str())) {
        LOG_INFO(L"Deleted oldest image: " + oldest.filePath);
    } else {
        LOG_WARNING(L"Failed to delete file: " + oldest.filePath);
    }

    m_images.pop_front();
    return true;
}

bool ImageStorage::EnsureDirectoryExists() {
    DWORD attr = GetFileAttributesW(m_storagePath.c_str());
    if (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY)) {
        return true;
    }

    return SHCreateDirectoryExW(nullptr, m_storagePath.c_str(), nullptr) == ERROR_SUCCESS ||
           GetLastError() == ERROR_ALREADY_EXISTS;
}

std::wstring ImageStorage::GenerateTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);

    std::tm tm;
    localtime_s(&tm, &time);

    std::wstringstream ss;
    ss << std::put_time(&tm, L"%Y%m%d_%H%M%S");

    return ss.str();
}

} // namespace Ephemery
