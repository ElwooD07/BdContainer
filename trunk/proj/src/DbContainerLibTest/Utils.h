#pragma once

void RefreshLog();

void DatabaseCreate();
void DatabaseRemove();

void DatabaseConnect();
void DatabaseDisconnect();

bool DatabasePrepare(); // create + connect + clear

void ShowExceptionMessages();