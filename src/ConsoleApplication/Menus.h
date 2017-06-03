#pragma once

#include "Wrapper.h"

bool MenuContainer();
bool MenuConnect();
bool MenuCreate();
void MenuDisconnect();
void SubMenuDisconnect();
void MenuClear();

void MenuFolder();
void MenuElementEdit(const ElementInfo &info);
void MenuFile();
void MenuStream();