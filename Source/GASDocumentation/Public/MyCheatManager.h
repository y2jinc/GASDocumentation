// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "MyCheatManager.generated.h"

struct FMusicContext
{
	FName Key = NAME_None;
	bool bPlayed = false;
	bool bStoped = false;
	bool bPaused = false;

	void Play();
	void Resume();
	void Stop(bool bInPause);
	bool IsPlaying() const;
	bool IsStopped() const;
	bool IsPaused() const;

	void DisplayInfo();
};

struct FMusicContextContainer
{
public:
	explicit FMusicContextContainer(const int32 InPriority) :
		Priority(InPriority) {}

	bool IsActivate() const;
	bool GetPauseOrStopOnPlay() const;
	int32 GetPriority() const;
	const FMusicContext* GetMusicContext(const FName& InKey) const;
	
	void Play();
	void Stop(const FName& InKey, bool bInPause);
	void StopAll(bool bInPause);

	bool operator<(const FMusicContextContainer& Other) const
	{
		return GetPriority() < Other.GetPriority() && IsActivate();
	}

	void AddContext(FName InKey, int32 InPriority);
	bool StopContext(const FName& InKey);
	void DisplayInfos();
	
private:
   	int32 Priority = 0;
   	bool bPaused = false;
	bool bStopped = false;
	TArray<TSharedPtr<FMusicContext>> MusicContexts;
};

/**
 * 
 */
UCLASS()
class GASABILITYDEMO_API UMyCheatManager : public UCheatManager
{
	GENERATED_BODY()
	
	UFUNCTION(exec)
	void TestCheatInit();

	UFUNCTION(exec)
	void TestPlayNode(FName InKey, int32 Priority);
	
	UFUNCTION(exec)
	void TestStopNode(FName InKey, int32 InPriority);

	UFUNCTION(exec)
   	void TestReleaseMusic();
	
	UFUNCTION(exec)
	void DisplayNodeInfos();

	void SortMusicContainers();
	void UpdateNodeContext();

private:
	TArray<TSharedPtr<FMusicContextContainer>> MusicContextContainers;
	TWeakPtr<FMusicContextContainer> ActivateTopPriorityMusicContainer;
};
