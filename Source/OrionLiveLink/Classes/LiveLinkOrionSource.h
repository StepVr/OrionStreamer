// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ILiveLinkSource.h"
#include "LiveLinkVRPNStream.h"

class ILiveLinkClient;
struct FLiveLinkPongMessage;
struct FLiveLinkSubjectDataMessage;
struct FLiveLinkSubjectFrameMessage;
struct FLiveLinkHeartbeatMessage;
struct FLiveLinkClearSubject;

class FLiveLinkOrionSource : public ILiveLinkSource
{
public:

	FLiveLinkOrionSource(const FText& InSubjectName, const FText& InSourceMachineName, int InPortNumber, bool InYup)
		: PortNumber(InPortNumber)
		, SubjectName(InSubjectName)
		, SourceMachineName(InSourceMachineName)
		, Yup(InYup)
	{}

	~FLiveLinkOrionSource()
	{

	}

	virtual void ReceiveClient(ILiveLinkClient* InClient, FGuid InSourceGuid);


	virtual bool IsSourceStillValid();

	virtual bool RequestSourceShutdown();

	virtual FText GetSourceType() const { return SubjectName; }
	virtual FText GetSourceMachineName() const { return SourceMachineName; }
	virtual FText GetSourceStatus() const { return SourceStatus; }

private:


	void HandleSubjectFrame(const TSharedPtr<FLiveLinkVRPNChar> character);

	ILiveLinkClient* Client;

	// Our identifier in LiveLink
	FGuid SourceGuid;


	ULiveLinkVRPNStream* DataStream;


	FText SubjectName;
	FText SourceMachineName;
	FText SourceStatus;
	bool Yup;
	int PortNumber;

};