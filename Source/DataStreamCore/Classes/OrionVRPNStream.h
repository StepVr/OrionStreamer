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

#include "Runtime/Core/Public/Async/AsyncWork.h"
#include "HAL/Runnable.h"
#include "OrionVRPNStream.generated.h"

class vrpn_Connection;
struct FOrionVRPNChar;
class UOrionVRPNStream;

UENUM()
enum ELAResult {
	ELASuccess,
	ELAError,
};

// Represents a binding of an Streamer subject to a specific skeleton.
// Resolves UE4 bone indices and stores them.

class ORIONDATASTREAMCORE_API FOrionDataStreamToSkeletonBinding
{
public:

	// Default constructor.
	FOrionDataStreamToSkeletonBinding();
	~FOrionDataStreamToSkeletonBinding();

	// Bind the given ViconStreamSubject to the given skeleton and store the result.
	// Since the member properties are not stored on file, this should be called
	// from runtime each time, to set up the members.
	bool BindToSkeleton(UOrionVRPNStream* Stream, const FString& subject,class USkeleton* skeleton = nullptr, class USkeletalMeshComponent* component = nullptr);

	bool UpdatePose(TArray<FTransform>& outPose);
	float GetFigureScale();

	bool IsBound();

	const FString& GetSubjectName();;

	// Access the UE4 bone index given the segment index.
	int32 GetUE4BoneIndex(int32 SegmentIndex) const;

	int32 GetNumberOfBones() const;

	void GetUE4BoneIndices(TArray<int32>& InBondsID);
	void SetUE4BoneIndices(TArray<int32>& InBondsID);
private:

	UOrionVRPNStream* mStream;

	FString mSubjectName;

	// Cache the UE4 bone indices corresponding to IKinema segments.
	TArray<int32> UE4BoneIndices;

	//Flag to make sure we are bound to a valid stream
	bool mBound;
};

UCLASS()
class UOrionVRPNStream : public UObject, public FRunnable
{
	GENERATED_UCLASS_BODY()

	TSharedPtr<vrpn_Connection> m_socket;	
	TMap<int, FString> AvatarNameMap;
	TMap<int, TSharedPtr<FOrionVRPNChar>> characters;
	
	FString _server_ip;
	uint16_t m_port;
	
	
	bool KillThread;
	int32 RefCount;
	int iDataPort;
	bool is_realtime;
	bool bIsInitialized;
	
	FRunnableThread* Thread;
	FCriticalSection section;
	FCriticalSection SocketSection;
	static TMap<FString, UOrionVRPNStream*> Streams;
	int GetSegmentIndex(int SubjectIndex, FString SegmentName);
	int GetSubjectIndex(const FString& SubjectName);
	float GetSubjectScale(const FString& SubjectName);
	uint32 Run() override;
public:

	virtual ~UOrionVRPNStream();;

	//Initialize the Socket and parset
	bool InitialiseCharacter(int senderId, const FString& senderName);

	const FString& GetServerIP();;
	TSharedPtr<vrpn_Connection>& GetSocket();
	bool isConnected();
	ELAResult GetFrame();

	bool GetPoseForSubject(FOrionDataStreamToSkeletonBinding* skeletonBinding, FString& subjectName, TArray<FTransform>& outPose);
	ELAResult GetSegmentCountForSubject(const FString& name, int& count);
	ELAResult GetSegmentNameForSubject(const FString& subjectName, int index, FString& segName);
	ELAResult GetSegmentParentNameForSubject(const FString& subjectName, FString segName, FString& parentSegName);

	static  UOrionVRPNStream* Get(const FString& serverIP);
	static  void Remove(const FString serverIP);
	ELAResult Connect(FString server, int portNumber = 3883);
	ELAResult Disconnect();

	void Destory();
	/* Begin: UObject Interface*/
	void BeginDestroy() override;
	void FinishDestroy() override;
	/*End: UObject Interface*/
};

