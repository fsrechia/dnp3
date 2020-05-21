/**************************************************************************
This function illustrates how to compute the CRC and check it for a
received data block.
**************************************************************************/

int dnp3_message_rx() {
    short idx; // Index
    unsigned char dataBlock[18]; // Array to hold received data block
    short blockSize; // Size of data block, not including CRC octets
    unsigned short crc; // 16-bit check code (crc accumulator)
    
    // ...
    // Insert code here to receive contents of dataBlock and blockSize;
    
    // ...
    // Compute check code for data in received block

    crc = 0; // Initialize
    for (idx = 0; idx < blockSize; idx++)
    computeCRC(dataBlock[idx],&crc);
    crc = ~crc; // Invert
    
    // Check CRC at end of block
    if (dataBlock[idx++] == (unsigned char)crc &&
    dataBlock[idx] == (unsigned char)(crc >> 8))
    {
        // Block received without error
        // ...
        // Insert code here to process good block
        // ...
    }
    else
    {
        // Error discovered
        // ...
        // Insert error processing code here
        // ...
    }
    // ...
    return 0;
}