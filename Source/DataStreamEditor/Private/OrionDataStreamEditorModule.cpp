/* Copyright (C) 2006-2017, IKinema Ltd. All rights reserved.
*
* IKinema API Library; SDK distribution
*
* This file is part of IKinema LiveAction project http://www.ikinema.com
*
* Your use and or redistribution of this software in source and / or binary form, with or without
* modification, is subject to:
* (i) your ongoing acceptance of and compliance with the terms and conditions of
* the IKinema License Agreement; and
*
* (ii) your inclusion of this notice in any version of this software that you use
* or redistribute.
*
*
* A copy of the IKinema License Agreement is available by contacting
* IKinema Ltd., http://www.ikinema.com, support@ikinema.com
*
*/

#include "OrionDataStreamEditorModule.h"
#include "OrionDataStreamEditorPrivatePCH.h"
#include "ModuleManager.h"


/**
* DataStreamEditor module implementation
*/

void FOrionDataStreamEditorModule::StartupModule()
{
	
}

void FOrionDataStreamEditorModule::ShutdownModule()
{
	
}

DEFINE_LOG_CATEGORY(LogOrionDataStreamEditor);
IMPLEMENT_MODULE(FOrionDataStreamEditorModule, OrionDataStreamEditor);

