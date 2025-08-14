#include <iostream>

#include <occa.hpp>
//remove some of the excess output and replace it with asserts 

//remove throw exceptions, replace with return 1; and error message 


int main(int argc, const char  *  * argv) {

  int entries = 12;

  float  * a  = new float[entries];
  float  * b  = new float[entries];
  float  * c  = new float[entries];
  float  * check  = new float[entries];

  for (int i = 0; i < entries; ++i) {
    a[i]  = 1 * (i+1);
    b[i]  = 2 * (i+1);
    c[i]  = 3 * (i+1);
  }

  occa::device device({{"mode", "Serial"}});

  occa::json properties;
  properties["verbose"] = false;


  occa::memoryPool memPool = device.createMemoryPool(properties);

  int alignment = memPool.alignment();


  std::cout << "Mempool Creation: alignment = " << alignment << std::endl;
  if (static_cast<size_t>(entries) > alignment/sizeof(float)) {
    std::cerr << "Example assumes vector lengths are less than mempool alignment." << std::endl;
    return 1;
  }

  std::cout << "Memory pool: Size = " << memPool.size() << " bytes"
            << ", Reserved = " << memPool.reserved() << " bytes"
            << ", in " << memPool.numReservations() << " reservations" << std::endl;

  std::cout << "First reservation: " << entries * sizeof(float) << " bytes\n" << std::endl;
  // Make a reservation from memory pool
  // Memory pool is a single allocation, and consists of just o_ab
  /* 
      |====o_a====|
   */
  occa::memory o_a = memPool.reserve<float>(entries);

  // Fill buffer
  o_a.copyFrom(a);

  std::cout << "Memory pool: Size = " << memPool.size() << " bytes"
            << ", Reserved = " << memPool.reserved() << " bytes"
            << ", in " << memPool.numReservations() << " reservations" << std::endl;


  // Unallocated occa::memory
  occa::memory o_c;

  // New scope
  {
    std::cout << "\n======Slicing Memory (no resize)" << std::endl;
    /* Slicing o_ab will not trigger reallocation or
      increase memoryPool's reservation size */
    /* 
      |====o_a=====|
      |a_h1=||a_h2=|
     */
    occa::memory o_a_half1 = o_a.slice(0, entries/2);
    occa::memory o_a_half2 = o_a.slice(entries/2);

    std::cout << "Memory pool: Size = " << memPool.size() << " bytes"
                   << ", Reserved = " << memPool.reserved() << " bytes"
                   << ", in " << memPool.numReservations() << " reservations" << std::endl;

    // Check the contents are what we expect
    o_a_half1.copyTo(check);
    for (int i = 0; i < entries/2; ++i) {
      if (!occa::areBitwiseEqual(check[i], a[i])) {
        return 1;
      }
    }

    o_a_half2.copyTo(check);
    for (int i = 0; i < entries/2; ++i) {
      if (!occa::areBitwiseEqual(check[i], a[i+entries/2])) {
        return 1;
      }
    }

    std::cout << "Second reservation: " << entries * sizeof(float) << " bytes" << std::endl;
    // Trigger a resize by requesting a new reservation:
    /* 
      |====o_a=====||====o_b=====|
      |a_h1=||a_h2=|
     */
    occa::memory o_b = memPool.reserve<float>(entries);

    // Fill buffer
    o_b.copyFrom(b);


    // Check o_a still has its data after resize
    o_a.copyTo(check);
    for (int i = 0; i < entries; ++i) {
      if (!occa::areBitwiseEqual(check[i], a[i])) {
        return 1;
      }
    }

    o_a_half1.copyTo(check);
    for (int i = 0; i < entries/2; ++i) {
      if (!occa::areBitwiseEqual(check[i], a[i])) {
        return 1;
      }
    }

    o_a_half2.copyTo(check);
    for (int i = 0; i < entries/2; ++i) {
      if (!occa::areBitwiseEqual(check[i], a[i+entries/2])) {
        return 1;
      }
    }

    //Destroy slices
    std::cout << "\n======Destroy slices" << std::endl;
    o_a_half1.free();
    o_a_half2.free();

    std::cout << "Memory pool: Size = " << memPool.size() << " bytes"
              << ", Reserved = " << memPool.reserved() << " bytes"
              << ", in " << memPool.numReservations() << " reservations" << std::endl;

    std::cout << "\n======Make a third reservation: " << entries * sizeof(float) << " bytes" << std::endl;
    // Trigger another resize by reserving the outer-scope mem
    /* 
      |====o_a=====||====o_b=====||====o_c=====|
      |a_h1=||a_h2=|
     */
    o_c = memPool.reserve<float>(entries);

    // Fill buffer
    o_c.copyFrom(c);

    std::cout << "Memory pool: Size = " << memPool.size() << " bytes"
              << ", Reserved = " << memPool.reserved() << " bytes"
              << ", in " << memPool.numReservations() << " reservations" << std::endl;

    // Check o_a and o_b have data after resize
    o_a.copyTo(check);
    for (int i = 0; i < entries; ++i) {
      if (!occa::areBitwiseEqual(check[i], a[i])) {
        return 1;
      }
    }

    o_b.copyTo(check);
    for (int i = 0; i < entries; ++i) {
      if (!occa::areBitwiseEqual(check[i], b[i])) {
        return 1;
      }
    }
  }

  std::cout << "\n======Free second reservation" << std::endl;
  // o_b leaves scope and is destroyed. This leaves a 'hole' in the memory pool
  /* 
    |====o_a=====||------------||====o_c=====|
   */
  std::cout << "Memory pool: Size = " << memPool.size() << " bytes"
            << ", Reserved = " << memPool.reserved() << " bytes"
            << ", in " << memPool.numReservations() << " reservations" << std::endl;

  std::cout << "\n======Re-reserve " << entries * sizeof(float) << " bytes" << std::endl;
  // Request a new reservation, should fit in the hole and not trigger resize:
  /* 
    |====o_a=====||====o_b=====||====o_c=====|
   */
  occa::memory o_b = memPool.reserve<float>(entries);

  std::cout << "Memory pool: Size = " << memPool.size() << " bytes"
            << ", Reserved = " << memPool.reserved() << " bytes"
            << ", in " << memPool.numReservations() << " reservations" << std::endl;

  std::cout << "\n======Free again" << std::endl;
  // Freeing doesnt change the mempool size, only the reserved size
  /* 
    |====o_a=====||------------||====o_c=====|
   */
  o_b.free();
  std::cout << "Memory pool: Size = " << memPool.size() << " bytes"
            << ", Reserved = " << memPool.reserved() << " bytes"
            << ", in " << memPool.numReservations() << " reservations" << std::endl;

  std::cout << "\n======Reserve " << (entries + alignment/sizeof(float)) * sizeof(float)
            << " bytes" << std::endl;
  // Requesting a new reservation that  * doesn't *  fit in the hole triggers a resize.
  // The mempool is defragmented on resizing
  /* 
    |====o_a====||====o_c=====||========o_b=========|
   */
  o_b = memPool.reserve<float>(entries + alignment/sizeof(float));

  std::cout << "Memory pool: Size = " << memPool.size() << " bytes"
            << ", Reserved = " << memPool.reserved() << " bytes"
            << ", in " << memPool.numReservations() << " reservations" << std::endl;

  // Check o_a and o_c have data after resize
  o_a.copyTo(check);
  for (int i = 0; i < entries; ++i) {
    if (!occa::areBitwiseEqual(check[i], a[i])) {
      return 1;
    }
  }
  o_c.copyTo(check);
  for (int i = 0; i < entries; ++i) {
    if (!occa::areBitwiseEqual(check[i], c[i])) {
      return 1;
    }
  }

  std::cout << "\n======Free third reservation and shrink to fit" << std::endl;
  // Free a reserved memory then "re-size to fit"
  /* 
    |====o_a=====||------------||========o_b=========|
   */
  o_c.free();

  /* 
    |====o_a=====||========o_b=========|
   */
  memPool.shrinkToFit();

  std::cout << "Memory pool: Size = " << memPool.size() << " bytes"
            << ", Reserved = " << memPool.reserved() << " bytes"
            << ", in " << memPool.numReservations() << " reservations" << std::endl;

  std::cout << "\n======Set alignment to " << 4 * memPool.alignment()
            << " bytes" << std::endl;
  // Set the alignment of the memory pool (triggers a re-allcation)
  memPool.setAlignment(4 * memPool.alignment());

  std::cout << "Memory pool: Size = " << memPool.size() << " bytes"
            << ", Reserved = " << memPool.reserved() << " bytes"
            << ", in " << memPool.numReservations() << " reservations" << std::endl;

  // Free host memory
  delete [] a;
  delete [] b;
  delete [] c;
  delete [] check;

  return 0;
}

