/**
 * @file action_resources.c
 * @brief Resource extraction utilities for Browser Sanity actions
 */

#include <browser_sanity.h>
#include <resource.h>
#include <windows.h>

/**
 * @brief Extracts the embedded msedge.exe binary from resources and writes it to disk
 * 
 * @param targetPath Path where the binary should be written
 * @return TRUE if successful, FALSE otherwise
 */
BOOL ExtractMsedgeBinary(const char* targetPath) {
    HMODULE hModule;
    HRSRC hResource;
    HGLOBAL hResourceData;
    void* pResourceData;
    DWORD resourceSize;
    FILE* outputFile;
    
    // Get handle to current module
    hModule = GetModuleHandle(NULL);
    if (!hModule) {
        return FALSE;
    }
    
    // Find the embedded msedge.exe resource
    hResource = FindResource(hModule, MAKEINTRESOURCE(IDR_MSEDGE_BINARY), RT_RCDATA);
    if (!hResource) {
        return FALSE;
    }
    
    // Get resource size
    resourceSize = SizeofResource(hModule, hResource);
    if (resourceSize == 0) {
        return FALSE;
    }
    
    // Load the resource
    hResourceData = LoadResource(hModule, hResource);
    if (!hResourceData) {
        return FALSE;
    }
    
    // Lock the resource to get a pointer to the data
    pResourceData = LockResource(hResourceData);
    if (!pResourceData) {
        return FALSE;
    }
    
    // Create/open the target file for writing
    outputFile = fopen(targetPath, "wb");
    if (!outputFile) {
        return FALSE;
    }
    
    // Write the resource data to the file
    size_t bytesWritten = fwrite(pResourceData, 1, resourceSize, outputFile);
    fclose(outputFile);
    
    // Check if all bytes were written successfully
    return (bytesWritten == resourceSize);
}