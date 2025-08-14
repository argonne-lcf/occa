#include <iostream>
#include <occa.hpp>

int main(int argc, const char **argv) {
    int chunks = 6; 
    int chunkSize = 64;
    int entries = chunks * chunkSize;

    std::cout << "Number of entries: " << entries << std::endl; 

    float *a = new float[entries];
    for (int i = 0; i < entries; ++i) {
        a[i] = i+1; // we will be summing the integers from 1 to entries
    }


    occa::device device({{"mode", "Serial"}});

    //Create two nonblocking streams so that we can run submit async kernels/operations
    occa::json streamProps({
        {"nonblocking", true},
    });
    occa::stream streamCopy = occa::createStream(streamProps);
    occa::stream streamKernel = occa::createStream(streamProps);

    // Allocate memory for chunks to be used for transfers and reductions
    // Note: if chunk size is less than half the entry size, this saves device memory 
    occa::memory o_chunk[2];
    for (int i = 0; i < 2; ++i)
        o_chunk[i] = occa::malloc<float>(chunkSize);

    // Allocate memory for reduction results for each chunk
    occa::memory o_results = occa::malloc<float>(chunks);

    // Build the kernel, including a define for the block parameter to be the chunkSize (allowing one kernel call per chunk)
    occa::properties reductionProps;
    reductionProps["defines/block"] = chunkSize;
    occa::kernel sumReduce = occa::buildKernel("reduction.okl", "reduction", reductionProps);



    // Make copies asynchronous so that we can overlap the copy with kernel execution
    occa::json async_copy({{"async", true}});


    // Prepare the first chunk for the first loop iteration
    occa::setStream(streamCopy);
    o_chunk[0].copyFrom(a, chunkSize, 0, async_copy);


    // Main reduction iteration
    for (int i = 0; i < chunks; ++i) {
        int cur = i % 2;
        int next = (i + 1) % 2;

        // Copy next chunk for future loop iteration
        if (i < chunks - 1) {
            occa::setStream(streamCopy);
            o_chunk[next].copyFrom(a + (i + 1) * chunkSize, chunkSize, 0, async_copy);
        }

        // Make sure copy for current chunk is finished before kernel launch using streamTags
        occa::setStream(streamCopy);
        occa::streamTag copyDone = occa::tagStream(); // todo explain streamtag more
        
        // Launch reduction on the previously copied chunk
        occa::setStream(streamKernel);
        occa::streamWait(streamKernel, copyDone); // only this stream waits for the tag, to make the entire device wait for a tag, use occa::waitFor(tag) todo make this used too? 
        // Call reduction kernel on the current chunk, giving it an index of o_results to store the result 
        sumReduce(chunkSize, o_chunk[cur], o_results+i);

    }

    // Wait for kernel stream to finish all work
    occa::setStream(streamKernel);
    streamKernel.finish();

    // Copy results to host
    float *hostResults = new float[chunks];
    o_results.copyTo(hostResults);

    // Do final reduction on host (can be done on device too)
    float totalSum = 0;
    for (int i = 0; i < chunks; ++i) {
        std::cout << "Partial sum at chunk " << i << ": " << hostResults[i] << std::endl; 
		totalSum += hostResults[i];
	}
    std::cout << "\n\n==================================\nTotal sum: " << totalSum << "\n";
	std::cout << "Expected value: " << entries * (entries+1) / 2 << std::endl;

    device.free();
    delete[] a;
    delete[] hostResults;
}