#pragma once
#include "ContainerAPI.h"

void RefreshLog();

void DatabaseCreate();
void DatabaseRemove();

void DatabaseConnect();
void DatabaseDisconnect();

bool DatabasePrepare(); // create + connect + clear

void AppendStream(std::ostream& strm, size_t size);
std::fstream CreateStream(size_t size);
void RewindStream(std::istream& strm);

unsigned int PrepareContainerForPartialWriteTest(dbc::ContainerGuard container, bool transactionalWrite); // returns cluster size

void ShowExceptionMessages();