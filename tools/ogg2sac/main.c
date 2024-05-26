#define _INCLUDE_TOKEN(p0, p1) <p0/p1>

#if set_compile == 0
  #define FAN_INCLUDE_PATH C:/libs/fan/include
#elif set_compile == 1
  #define FAN_INCLUDE_PATH /usr/include
#else
  #error ?
#endif

#ifndef WITCH_INCLUDE_PATH
  #define WITCH_INCLUDE_PATH WITCH
#endif
#include _INCLUDE_TOKEN(WITCH_INCLUDE_PATH,WITCH.h)

#include _WITCH_PATH(IO/IO.h)
#include _WITCH_PATH(FS/FS.h)
#include _WITCH_PATH(A/A.h)
#include _WITCH_PATH(IO/print.h)

void WriteOut(const char *format, ...){
  IO_fd_t fd_stdout;
  IO_fd_set(&fd_stdout, FD_OUT);
  va_list argv;
  va_start(argv, format);
  IO_vprint(&fd_stdout, format, argv);
  va_end(argv);
}

#pragma pack(push, 1)

typedef struct{
  uint8_t Version;
  uint8_t HeaderType;
  uint64_t GranulePosition;
  uint32_t BitstreamSerialNumber;
  uint32_t PageSequenceNumber;
  uint32_t Checksum;
  uint8_t PageSegments;
}OggHead_t;

typedef struct{
  uint8_t Version;
  uint8_t ChannelCount;
  uint16_t PreSkip;
  uint32_t InputSampleRate;
  sint16_t OutputGain;
  uint8_t ChannelMapping;
}OpusHead_t;

#pragma pack(pop)

typedef struct{
  uint32_t filler;
}SAC_t;

void SAC_close(SAC_t *sac){
  
}
void SAC_open(SAC_t *sac){
  
}

void ogg2sac(SAC_t *sac, uint8_t *Data, uintptr_t Size, const char *DestinationPath){
  #define OggMagicBytesSize 4
  const uint8_t OggMagicBytes[OggMagicBytesSize] = {'O', 'g', 'g', 'S'};
  #define OpusMagicBytesSize 8
  const uint8_t OpusMagicBytes[OpusMagicBytesSize] = {'O', 'p', 'u', 's', 'H', 'e', 'a', 'd'};

  #define tbs(p0) \
    if(DataIndex + (p0) > Size){ \
      WriteOut("tbs(%lu) failed\n", (uint64_t)(p0)); \
      return; \
    }
  uintptr_t DataIndex = 0;

  uint8_t Segment0;
  { /* + ogg head */
    { /* + sac check */
      tbs(1);
      if(Data[DataIndex] == 0xff){
        /* its a sac file */
        return;
      }
    } /* - sac check */
    tbs(OggMagicBytesSize)
    for(uint32_t i = 0; i < OggMagicBytesSize; i++){
      if(Data[DataIndex] != OggMagicBytes[i]){
        WriteOut("didnt find OggS\n");
        return;
      }
      DataIndex++;
    }

    tbs(sizeof(OggHead_t))
    OggHead_t OggHead = *(OggHead_t *)&Data[DataIndex];
    DataIndex += sizeof(OggHead_t);

    if(OggHead.PageSegments != 1){
      WriteOut("in need of w\n");
      PR_abort();
    }

    tbs(1)
    Segment0 = *(uint8_t *)&Data[DataIndex];
    DataIndex += 1;
  } /* - ogg head */

  OpusHead_t OpusHead;
  { /* + opus head */
    if(Segment0 < sizeof(OpusHead_t)){
      WriteOut("Segment0 is smaller than sizeof(OpusHead_t)\n");
      return;
    }
    tbs(OpusMagicBytesSize)
    for(uint32_t i = 0; i < OpusMagicBytesSize; i++){
      if(Data[DataIndex] != OpusMagicBytes[i]){
        WriteOut("didnt find opushead\n");
        return;
      }
      DataIndex++;
    }

    tbs(sizeof(OpusHead_t))
    OpusHead = *(OpusHead_t *)&Data[DataIndex];
    DataIndex += sizeof(OpusHead_t);

    if(OpusHead.ChannelMapping != 0){
      WriteOut(
        "who gets this first fixes it first\n"
        "rfc7845 section-5.1.1\n"
      );
      PR_abort();
    }
  } /* - opus head */

  uint8_t Segment1;
  { /* + ogg head */
    tbs(OggMagicBytesSize)
    for(uint32_t i = 0; i < OggMagicBytesSize; i++){
      if(Data[DataIndex] != OggMagicBytes[i]){
        WriteOut("didnt find OggS\n");
        return;
      }
      DataIndex++;
    }

    tbs(sizeof(OggHead_t))
    OggHead_t OggHead = *(OggHead_t *)&Data[DataIndex];
    DataIndex += sizeof(OggHead_t);

    if(OggHead.PageSegments != 1){
      WriteOut("in need of w\n");
      PR_abort();
    }

    tbs(1)
    Segment1 = *(uint8_t *)&Data[DataIndex];
    DataIndex += 1;
  } /* - ogg head */

  { /* + opus tags */
    if(Segment1 < OpusMagicBytesSize){
      WriteOut("Segment1 smaller than OpusTags\n");
      return;
    }
    tbs(OpusMagicBytesSize)
    uint8_t OpusTagsBytes[OpusMagicBytesSize] = {'O', 'p', 'u', 's', 'T', 'a', 'g', 's'};
    for(uint32_t i = 0; i < OpusMagicBytesSize; i++){
      if(Data[DataIndex] != OpusTagsBytes[i]){
        WriteOut("didnt find opustags\n");
        return;
      }
      DataIndex++;
    }

    /* we are passing whole */
    uint8_t seek = Segment1 - OpusMagicBytesSize;
    tbs(seek);
    DataIndex += seek;
  } /* - opus tags */

  /* atomic file */
  FS_file_t af;
  sint32_t err = FS_file_opentmp(&af);
  if(err != 0){
    WriteOut("good luck to backend owner");
    PR_abort();
  }

  { /* SAC head */
    uint8_t SacSign = 0xff;
    FS_file_write(&af, &SacSign, sizeof(uint8_t));
    uint16_t FillerChecksum = 0;
    FS_file_write(&af, &FillerChecksum, sizeof(FillerChecksum));
    FS_file_write(&af, &OpusHead.ChannelCount, sizeof(OpusHead.ChannelCount));

    uint32_t zero = 0;

    /* BeginCut */
    FS_file_write(&af, &zero, sizeof(uint16_t));
    /* EndCut */
    FS_file_write(&af, &zero, sizeof(uint16_t));

    /* TotalSegments */
    FS_file_write(&af, &zero, sizeof(uint32_t));
  }

  uint32_t TotalSegments = 0;
  { /* + total segments */
    uintptr_t DataIndex0 = DataIndex;
    OggHead_t StreamOggHead;
    uint32_t TotalPackets = 0;
    while(DataIndex != Size){
      uint8_t *SegmentTable;
      { /* + ogg head */
        tbs(OggMagicBytesSize)
        for(uint32_t i = 0; i < OggMagicBytesSize; i++){
          if(Data[DataIndex] != OggMagicBytes[i]){
            WriteOut("didnt find OggS\n");
            return;
          }
          DataIndex++;
        }

        tbs(sizeof(OggHead_t))
        StreamOggHead = *(OggHead_t *)&Data[DataIndex];
        DataIndex += sizeof(OggHead_t);

        tbs(StreamOggHead.PageSegments * sizeof(uint8_t))
        SegmentTable = (uint8_t *)&Data[DataIndex];
        DataIndex += StreamOggHead.PageSegments * sizeof(uint8_t);

        FS_file_write(&af, SegmentTable, StreamOggHead.PageSegments * sizeof(uint8_t));
      } /* - ogg head */

      TotalSegments += StreamOggHead.PageSegments;
      for(uint32_t i = 0; i < StreamOggHead.PageSegments; i++){
        /* Segment Size */
        uint8_t SS = SegmentTable[i];

        if(SS != 0xff){
          TotalPackets++;
        }

        tbs(SS)
        DataIndex += SS;
      }
    }

    uint64_t EndCut = (uint64_t)TotalPackets * 960;
    if(EndCut < StreamOggHead.GranulePosition){
      WriteOut("[Warning] - GranulePosition is bigger than packet lengths. a bit broken opus?\n");
      EndCut = StreamOggHead.GranulePosition;
    }
    EndCut -= StreamOggHead.GranulePosition;
    if(EndCut > 0xffff){
      WriteOut("EndCut is toooooo big %llx\n", EndCut);
      PR_abort();
    }
    FS_file_seek(&af, 6, FS_file_seek_Begin);
    FS_file_write(&af, &EndCut, sizeof(uint16_t));

    if(OpusHead.PreSkip >= (uint64_t)TotalPackets * 960){
      WriteOut("PreSkip is toooooo big %llx\n", OpusHead.PreSkip);
      PR_abort();
    }
    FS_file_seek(&af, 4, FS_file_seek_Begin);
    FS_file_write(&af, &OpusHead.PreSkip, sizeof(uint16_t));

    DataIndex = DataIndex0;
  } /* - total segments */

  FS_file_seek(&af, 8, FS_file_seek_Begin);
  FS_file_write(&af, &TotalSegments, sizeof(TotalSegments));

  FS_file_seek(&af, 0, FS_file_seek_End);

  while(DataIndex != Size){
    OggHead_t StreamOggHead;
    uint8_t *SegmentTable;
    { /* + ogg head */
      tbs(OggMagicBytesSize)
      for(uint32_t i = 0; i < OggMagicBytesSize; i++){
        if(Data[DataIndex] != OggMagicBytes[i]){
          WriteOut("didnt find OggS\n");
          return;
        }
        DataIndex++;
      }

      tbs(sizeof(OggHead_t))
      StreamOggHead = *(OggHead_t *)&Data[DataIndex];
      DataIndex += sizeof(OggHead_t);

      tbs(StreamOggHead.PageSegments * sizeof(uint8_t))
      SegmentTable = (uint8_t *)&Data[DataIndex];
      DataIndex += StreamOggHead.PageSegments * sizeof(uint8_t);
    } /* - ogg head */

    for(uint32_t i = 0; i < StreamOggHead.PageSegments; i++){
      /* Segment Size */
      uint8_t SS = SegmentTable[i];

      tbs(SS)
      FS_file_write(&af, &Data[DataIndex], SS);
      DataIndex += SS;
    }
  }

  err = FS_file_rename(&af, DestinationPath);
  if(err != 0){
    WriteOut("rename failed\n");
    return;
  }

  #undef tbs

  #undef OpusMagicBytesSize
  #undef OggMagicBytesSize
}

int main(int argc, char **argv){
  SAC_t sac;
  SAC_open(&sac);

  for(int ai = 1; ai < argc; ai++){
    FS_file_t f;
    sint32_t err;
    err = FS_file_open(argv[ai], &f, O_RDONLY);
    if(err != 0){
      WriteOut("file failed to open %s\n", argv[ai]);
      continue;
    }
    IO_fd_t fd;
    FS_file_getfd(&f, &fd);
    IO_stat_t s;
    err = IO_fstat(&fd, &s);
    if(err != 0){
      WriteOut("file failed to get stat %s\n", argv[ai]);
      FS_file_close(&f);
      continue;
    }
    IO_off_t TotalSize = IO_stat_GetSizeInBytes(&s);
    uint8_t *Data = (uint8_t *)A_resize(0, TotalSize);
    IO_ssize_t Size = FS_file_read(&f, Data, TotalSize);
    if(Size != TotalSize){
      WriteOut("file failed to read %s\n", argv[ai]);
      A_resize(Data, 0);
      FS_file_close(&f);
      continue;
    }
    FS_file_close(&f);

    ogg2sac(&sac, Data, Size, argv[ai]);

    A_resize(Data, 0);
  }

  SAC_close(&sac);

  return 0;
}
