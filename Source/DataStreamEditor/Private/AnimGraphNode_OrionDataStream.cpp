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

#include "AnimGraphNode_OrionDataStream.h"
#include "OrionDataStreamEditorPrivatePCH.h"

#include "CompilerResultsLog.h"

UAnimGraphNode_OrionDataStream::UAnimGraphNode_OrionDataStream(const FObjectInitializer&  PCIP)
	: Super(PCIP)
{
}

FText UAnimGraphNode_OrionDataStream::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	
	return FText::FromString("Orion Stream");
}

FLinearColor UAnimGraphNode_OrionDataStream::GetNodeTitleColor() const
{
	return FLinearColor(0.75f, 0.75f, 0.75f);
}

FText UAnimGraphNode_OrionDataStream::GetTooltipText() const
{
	return FText::FromString("Retrieves streamed skeletal animation from Orion");
}

FString UAnimGraphNode_OrionDataStream::GetNodeCategory() const
{
	return FString("IKinema");
}

void UAnimGraphNode_OrionDataStream::ValidateAnimNodeDuringCompilation(class USkeleton* ForSkeleton, class FCompilerResultsLog& MessageLog)
{
}