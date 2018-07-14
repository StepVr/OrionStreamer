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

#pragma once
#include "OrionVRPNStream.h"
#include "StepVrServerModule.h"
#include "Animation/AnimNodeBase.h"
#include "AnimNode_OrionDataStream.generated.h"




USTRUCT(BlueprintType)
struct  ORIONDATASTREAMCORE_API FAnimNode_OrionDataStream : public FAnimNode_Base
{
	GENERATED_USTRUCT_BODY()

	// The input pose is segmented into two:
	// - The FK input pose that serves as a bias for the solver.
	// - The task targets appended at the end.
	FPoseLink InPose;
	
	UPROPERTY(EditAnywhere, Category=Server, meta=(PinShownByDefault))
	FName ServerName;

	UPROPERTY(EditAnywhere, Category=Server,meta=(PinShownByDefault))
	int32 PortNumber;

	UPROPERTY(EditAnywhere, Category = Server, meta = (PinShownByDefault))
	FName SubjectName;

	UPROPERTY(EditAnywhere, Category = Server, meta = (PinShownByDefault))
	bool IsLocalControlled = false;

	UPROPERTY(EditAnywhere, Category = Server, meta = (PinShownByDefault))
	int32 OwnerPlayerID = 0;

	UPROPERTY(EditAnywhere, Category = Server, meta = (PinShownByDefault,ToolTip = "Attemp to Reconnect?"))
	bool Reconnect;

	UPROPERTY(EditAnywhere, Category = Server, meta = (PinShownByDefault, ToolTip = "Orion Character has Y-up"))
	bool StreamYUp;

	UPROPERTY(EditAnywhere, Category = Server, meta = (PinShownByDefault, ToolTip = "Apply character scale from Orion"))
	bool ApplyOrionScale;
private:
	FString StrServerName;
	FString StrSubjectName;

public:	

	FAnimNode_OrionDataStream();
	~FAnimNode_OrionDataStream();
	void BindSkeleton(FAnimInstanceProxy* AnimInstanceProxy);
	void IntializeStreamer(FAnimInstanceProxy* AnimInstanceProxy);
	// Keep the binding here.
	FOrionDataStreamToSkeletonBinding mSkeletonBinding;

	
	// FAnimNode_Base interface
	void Initialize_AnyThread(const FAnimationInitializeContext& Context) override;
	void Update_AnyThread(const FAnimationUpdateContext& Context) override;
	void Evaluate_AnyThread(FPoseContext& Output) override;
	void CacheBones_AnyThread(const FAnimationCacheBonesContext & Context) override;
	// End of FAnimNode_Base interface

private:
	UOrionVRPNStream* Streamer;
	int numberOfAttempts;
	bool ReallyReconnect;
	int32 previousPort;

	IKinemaReplicateData	FrameCacheData;
	IKinemaReplicateData	FrameSendData;
};

