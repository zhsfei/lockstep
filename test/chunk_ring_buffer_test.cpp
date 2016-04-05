#include <string.h>
#include "orwell.h"
#include "lib/chunk_ring_buffer.h"

static void TestBasicWriteRead(ow_test_context Context) {
  ui8 RingBufferBlock[256];
  buffer RingBuffer = {
    .Addr = &RingBufferBlock,
    .Length = sizeof(RingBufferBlock)
  };
  chunk_ring_buffer Ring;
  InitChunkRingBuffer(&Ring, 2, RingBuffer);
  char InputBlock[] = { 'h', 'e', 'y', '\0' };
  const buffer Input = { .Addr = &InputBlock, .Length = 4 };
  ChunkRingBufferWrite(&Ring, Input);

  char OutputBlock[8];
  buffer Output = { .Addr = &OutputBlock, .Length = sizeof(OutputBlock) };
  memsize ReadLength = ChunkRingBufferRead(&Ring, Output);

  OW_AssertEqualInt(4, ReadLength);
  OW_AssertEqualStr("hey", OutputBlock);

  TerminateChunkRingBuffer(&Ring);
}

static void TestLoopingWriteRead(ow_test_context Context) {
  char Input0[] = { 'h', 'e', 'l', 'l', 'o', '\0' };
  char Input1[] = { 'w', 'h', 'a', 't', '\0' };
  char Input2[] = { 'i', 's', '\0' };
  char Input3[] = { 'u', 'p', '\0' };
  char Input4[] = { 'o', 'v', 'e', 'r', 't', 'h', 'e', 'r', 'e', '\0' };
  char* InputBufferBlocks[] = { Input0, Input1, Input2, Input3, Input4 };
  buffer Inputs[] = {
    { .Addr = InputBufferBlocks[0], .Length = strlen(InputBufferBlocks[0])+1 },
    { .Addr = InputBufferBlocks[1], .Length = strlen(InputBufferBlocks[1])+1 },
    { .Addr = InputBufferBlocks[2], .Length = strlen(InputBufferBlocks[2])+1 },
    { .Addr = InputBufferBlocks[3], .Length = strlen(InputBufferBlocks[3])+1 },
  };
  ui8 InputCount = sizeof(Inputs)/sizeof(Inputs[0]);

  ui8 RingBufferBlock[256];
  buffer RingBuffer = {
    .Addr = &RingBufferBlock,
    .Length = sizeof(RingBufferBlock)
  };
  chunk_ring_buffer Ring;
  InitChunkRingBuffer(&Ring, 5, RingBuffer);
  ChunkRingBufferWrite(&Ring, Inputs[0]);
  ChunkRingBufferWrite(&Ring, Inputs[1]);
  memsize ReadIndex = 0;
  memsize WriteIndex = 2;

  for(memsize I=0; I<200; I++) {
    char ResultBuffer[10];
    buffer Result = { .Addr = &ResultBuffer, .Length = sizeof(ResultBuffer) };
    memsize ReadLength = ChunkRingBufferRead(&Ring, Result);
    OW_AssertEqualInt(Inputs[ReadIndex].Length, ReadLength);
    OW_AssertEqualStr((const char*)Inputs[ReadIndex].Addr, (const char*)Result.Addr);

    ChunkRingBufferWrite(&Ring, Inputs[WriteIndex]);

    ReadIndex = (ReadIndex + 1) % InputCount;
    WriteIndex = (WriteIndex + 1) % InputCount;
  }

  TerminateChunkRingBuffer(&Ring);
}

static void TestEmpty(ow_test_context Context) {
  ui8 RingBufferBlock[128];
  buffer RingBuffer = { .Addr = RingBufferBlock, .Length = sizeof(RingBufferBlock) };
  chunk_ring_buffer Ring;
  InitChunkRingBuffer(&Ring, 4, RingBuffer);
  ui8 InputBlock[4];
  buffer Input = { .Addr = InputBlock, .Length = sizeof(InputBlock) };
  ChunkRingBufferWrite(&Ring, Input);

  char ResultBlock[8];
  buffer Result = { .Addr = ResultBlock, .Length = sizeof(ResultBlock) };
  memsize ReadLength = ChunkRingBufferRead(&Ring, Result);
  OW_AssertEqualInt(4, ReadLength);
  ReadLength = ChunkRingBufferRead(&Ring, Result);
  OW_AssertEqualInt(0, ReadLength);
  ReadLength = ChunkRingBufferRead(&Ring, Result);
  OW_AssertEqualInt(0, ReadLength);

  TerminateChunkRingBuffer(&Ring);
}

void SetupChunkRingBufferGroup(ow_suite *S) {
  ow_group_index G = OW_CreateGroup(S);
  OW_AddTest(S, G, TestBasicWriteRead);
  OW_AddTest(S, G, TestLoopingWriteRead);
  OW_AddTest(S, G, TestEmpty);
}
