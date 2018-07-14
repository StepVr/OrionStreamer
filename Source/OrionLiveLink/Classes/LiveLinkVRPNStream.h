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
#include "LiveLinkVRPNStream.generated.h"

class vrpn_Connection;
struct FLiveLinkVRPNChar;
class ULiveLinkVRPNStream;

UENUM()
enum ELAResult {
	ELASuccess,
	ELAError,
};

class FLiveLinkDataStreamToSkeletonBinding
{
public:

	// Default constructor.
	FLiveLinkDataStreamToSkeletonBinding();
	~FLiveLinkDataStreamToSkeletonBinding();

	// Bind the given ViconStreamSubject to the given skeleton and store the result.
	// Since the member properties are not stored on file, this should be called
	// from runtime each time, to set up the members.
	bool BindToSkeleton(ULiveLinkVRPNStream* Stream, const FString& subject, class USkeleton* skeleton = nullptr, class USkeletalMeshComponent* component = nullptr);

	bool UpdatePose(TArray<FTransform>& outPose);
	float GetFigureScale();

	bool IsBound();

	const FString& GetSubjectName();;

	// Access the UE4 bone index given the segment index.
	int32 GetUE4BoneIndex(int32 SegmentIndex) const;

	int32 GetNumberOfBones() const;
private:

	ULiveLinkVRPNStream* mStream;

	FString mSubjectName;

	// Cache the UE4 bone indices corresponding to IKinema segments.
	TArray<int32> UE4BoneIndices;

	//Flag to make sure we are bound to a valid stream
	bool mBound;
};


/**
* Template for handlers of one specific message type (via raw function pointers).
*
* @param MessageType The type of message to handle.
* @param HandlerType The type of the handler class.
*/


class IHandler
{
public:

	/**
	* Handles the specified message.
	*
	* @param Context The context of the message to handle.
	*/
	virtual void HandleMessage(const TSharedPtr<FLiveLinkVRPNChar> character) = 0;

public:

	/** Virtual destructor. */
	virtual ~IHandler() { }
};


template<typename HandlerType>
class TCharHandler : public IHandler
{
public:

	/** Type definition for function pointers that are compatible with this TRawMessageHandler. */
	typedef void (HandlerType::*FuncType)(const TSharedPtr<FLiveLinkVRPNChar>);

public:

	/**
	* Creates and initializes a new message handler.
	*
	* @param InHandler The object that will handle the messages.
	* @param InFunc The object's message handling function.
	*/
	TCharHandler(HandlerType* InHandler, FuncType InFunc)
		: Handler(InHandler)
		, Func(InFunc)
	{
		check(InHandler != nullptr);
	}

	/** Virtual destructor. */
	TCharHandler() { }

public:

	//~ IMessageHandler interface

	virtual void HandleMessage(const TSharedPtr<FLiveLinkVRPNChar> character) override
	{
		(Handler->*Func)(character);
	}

private:

	/** Holds a pointer to the object handling the messages. */
	HandlerType* Handler;

	/** Holds a pointer to the actual handler function. */
	FuncType Func;
};


UCLASS()
class ULiveLinkVRPNStream : public UObject, public FRunnable
{
	GENERATED_UCLASS_BODY()

	TSharedPtr<vrpn_Connection> m_socket;	
	TMap<int, FString> AvatarNameMap;
	TMap<int, TSharedPtr<FLiveLinkVRPNChar>> characters;
	
	FString _server_ip;
	uint16_t m_port;
	
	
	bool KillThread;
	int32 RefCount;
	int iDataPort;
	bool is_realtime;
	bool bIsInitialized;
	
	FRunnableThread* Thread;
	FCriticalSection section;
	static TMap<FString, ULiveLinkVRPNStream*> Streams;
	int GetSegmentIndex(int SubjectIndex, FString SegmentName);
	int GetSubjectIndex(const FString& SubjectName);
	float GetSubjectScale(const FString& SubjectName);
	uint32 Run() override;
public:

	virtual ~ULiveLinkVRPNStream();;

	//Initialize the Socket and parset
	bool InitialiseCharacter(int senderId, const FString& senderName);

	const FString& GetServerIP();;
	TSharedPtr<vrpn_Connection>& GetSocket();
	bool isConnected();
	ELAResult GetFrame();

	bool GetPoseForSubject(class FLiveLinkDataStreamToSkeletonBinding* skeletonBinding, FString& subjectName, TArray<FTransform>& outPose);
	ELAResult GetSegmentCountForSubject(const FString& name, int& count);
	ELAResult GetSegmentNameForSubject(const FString& subjectName, int index, FString& segName);
	ELAResult GetSegmentParentNameForSubject(const FString& subjectName, FString segName, FString& parentSegName);

	static  ULiveLinkVRPNStream* Get(const FString& serverIP);
	static  void Remove(const FString serverIP);
	ELAResult Connect(FString server, int portNumber = 3883);
	ELAResult Disconnect();

	void Destory();
	/* Begin: UObject Interface*/
	void BeginDestroy() override;
	void FinishDestroy() override;
	/*End: UObject Interface*/

	template<typename HandlerType>
	void SetCharacterCallBack(HandlerType* InHandler, typename TCharHandler<HandlerType>::FuncType HandlerFunc)
	{
		Handler = new TCharHandler<HandlerType>(InHandler,MoveTemp(HandlerFunc));//MakeShareable(new TCharHandler<HandlerType>(InHandler, MoveTemp(HandlerFunc)));
	}

	void RemoveCharacterCallBack()
	{
	//	Handler.Reset();
		Handler = nullptr;
	}
	//TSharedPtr<IHandler> Handler;
	IHandler* Handler;

};

