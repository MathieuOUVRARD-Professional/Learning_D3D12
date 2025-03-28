#include <Support/DStorageUsage.h>

//bufferSize in MB
void SetupDStorage(int bufferSize)
{
    DSTORAGE_QUEUE_DESC queueDesc = {};
    queueDesc.Capacity = DSTORAGE_MAX_QUEUE_CAPACITY;
    queueDesc.Priority = DSTORAGE_PRIORITY_HIGH;
    queueDesc.SourceType = DSTORAGE_REQUEST_SOURCE_FILE;
    queueDesc.Device = DXContext::Get().GetDevice();

    // Create the DStorage queue
    m_dStorage->SetStagingBufferSize(bufferSize * 1024 * 1024);
    m_dStorage->CreateQueue(&queueDesc, IID_PPV_ARGS(&m_queue));
}
