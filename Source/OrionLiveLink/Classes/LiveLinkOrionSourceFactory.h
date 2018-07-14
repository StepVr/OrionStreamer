// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "LiveLinkSourceFactory.h"
#include "LiveLinkOrionSourceFactory.generated.h"


class SMocapSourcePicker;

UCLASS()
class ULiveLinkOrionSourceFactory : public ULiveLinkSourceFactory
{
public:

	GENERATED_BODY()
	
	virtual FText GetSourceDisplayName() const;
	virtual FText GetSourceTooltip() const;

	virtual TSharedPtr<SWidget> CreateSourceCreationPanel();
	virtual TSharedPtr<ILiveLinkSource> OnSourceCreationPanelClosed(bool bMakeSource);

	TSharedPtr<SMocapSourcePicker> ActiveSourceEditor;
};