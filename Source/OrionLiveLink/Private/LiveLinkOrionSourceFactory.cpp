// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
#include "OrionLiveLinkPrivatePCH.h"
#include "LiveLinkOrionSourceFactory.h"
#include "LiveLinkOrionSource.h"
#include "LiveLinkOrionSourceEditor.h"

#define LOCTEXT_NAMESPACE "LiveLinkOrionSourceFactory"




FText ULiveLinkOrionSourceFactory::GetSourceDisplayName() const
{
	return LOCTEXT("SourceDisplayName", "Orion Source");
}

FText ULiveLinkOrionSourceFactory::GetSourceTooltip() const
{
	return LOCTEXT("SourceTooltip", "Creates a connection to a Orion VRPN based Live Link Source");
}

TSharedPtr<SWidget> ULiveLinkOrionSourceFactory::CreateSourceCreationPanel()
{
	if (!ActiveSourceEditor.IsValid())
	{
		SAssignNew(ActiveSourceEditor, SMocapSourcePicker);
	}
	return ActiveSourceEditor;
}

TSharedPtr<ILiveLinkSource> ULiveLinkOrionSourceFactory::OnSourceCreationPanelClosed(bool bMakeSource)
{
	//Clean up
	TSharedPtr<FLiveLinkOrionSource> NewSource = nullptr;

	if (bMakeSource && ActiveSourceEditor.IsValid())
	{
		//FProviderPollResultPtr Result = ActiveSourceEditor->GetSelectedSource();
		//if(Result.IsValid())
		{
			NewSource = MakeShareable( new FLiveLinkOrionSource(FText::FromString("Orion LiveLink"), FText::FromString(ActiveSourceEditor->GetServerIP()), ActiveSourceEditor->GetPortNumber(),ActiveSourceEditor->IsYup()));
		}
	}
	ActiveSourceEditor = nullptr;
	return NewSource;
}

#undef LOCTEXT_NAMESPACE