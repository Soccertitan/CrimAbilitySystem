// Copyright Soccertitan

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayTagContainer.h"
#include "Subsystems/WorldSubsystem.h"
#include "CrimGamePhaseSubsystem.generated.h"

class UGamePhaseGameplayAbility;

DECLARE_DYNAMIC_DELEGATE_OneParam(FCrimGamePhaseDynamicDelegate, const UGamePhaseGameplayAbility*, Phase);
DECLARE_DELEGATE_OneParam(FCrimGamePhaseDelegate, const UGamePhaseGameplayAbility* Phase);

DECLARE_DYNAMIC_DELEGATE_OneParam(FCrimGamePhaseTagDynamicDelegate, const FGameplayTag&, PhaseTag);
DECLARE_DELEGATE_OneParam(FCrimGamePhaseTagDelegate, const FGameplayTag& PhaseTag);

// Match rule for message receivers
UENUM(BlueprintType)
enum class EPhaseTagMatchType : uint8
{
	// An exact match will only receive messages with exactly the same channel
	// (e.g., registering for "A.B" will match a broadcast of A.B but not A.B.C)
	ExactMatch,

	// A partial match will receive any messages rooted in the same channel
	// (e.g., registering for "A.B" will match a broadcast of A.B as well as A.B.C)
	PartialMatch
};

/**
 * Subsystem for managing game phases using gameplay tags in a nested manner, which allows parent and child 
 * phases to be active at the same time, but not sibling phases.
 * Example: Game.Playing and Game.Playing.WarmUp can coexist, but Game.Playing and Game.ShowingScore cannot. 
 * When a new phase is started, any active phases that are not ancestors will be ended.
 * Example: If Game.Playing and Game.Playing.CaptureTheFlag are active when Game.Playing.PostGame is started, 
 * Game.Playing will remain active, while Game.Playing.CaptureTheFlag will end.
 */
UCLASS()
class CRIMABILITYSYSTEM_API UGamePhaseSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UGamePhaseSubsystem();

	void StartPhase(TSubclassOf<UGamePhaseGameplayAbility> PhaseAbility, FCrimGamePhaseDelegate PhaseEndedCallback = FCrimGamePhaseDelegate());

	//TODO Return a handle so folks can delete these.  They will just grow until the world resets.
	//TODO Should we just occasionally clean these observers up?  It's not as if everyone will properly unhook them even if there is a handle.
	void WhenPhaseStartsOrIsActive(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, const FCrimGamePhaseTagDelegate& WhenPhaseActive);
	void WhenPhaseEnds(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, const FCrimGamePhaseTagDelegate& WhenPhaseEnd);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, BlueprintPure = false, meta = (AutoCreateRefTerm = "PhaseTag"))
	bool IsPhaseActive(const FGameplayTag& PhaseTag) const;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Crim Ability System|Game Phase", meta = (DisplayName="Start Phase", AutoCreateRefTerm = "PhaseEnded"))
	void K2_StartPhase(TSubclassOf<UGamePhaseGameplayAbility> PhaseAbility, const FCrimGamePhaseDynamicDelegate& PhaseEnded);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Crim Ability System|Game Phase", meta = (DisplayName = "When Phase Starts or Is Active", AutoCreateRefTerm = "WhenPhaseActive"))
	void K2_WhenPhaseStartsOrIsActive(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, FCrimGamePhaseTagDynamicDelegate WhenPhaseActive);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Crim Ability System|Game Phase", meta = (DisplayName = "When Phase Ends", AutoCreateRefTerm = "WhenPhaseEnd"))
	void K2_WhenPhaseEnds(FGameplayTag PhaseTag, EPhaseTagMatchType MatchType, FCrimGamePhaseTagDynamicDelegate WhenPhaseEnd);

	void OnBeginPhase(const UGamePhaseGameplayAbility* PhaseAbility, const FGameplayAbilitySpecHandle PhaseAbilityHandle);
	void OnEndPhase(const UGamePhaseGameplayAbility* PhaseAbility, const FGameplayAbilitySpecHandle PhaseAbilityHandle);

private:

	struct FCrimGamePhaseEntry
	{
		FGameplayTag PhaseTag;
		FCrimGamePhaseDelegate PhaseEndedCallback;
	};

	TMap<FGameplayAbilitySpecHandle, FCrimGamePhaseEntry> ActivePhaseMap;

	struct FPhaseObserver
	{
		bool IsMatch(const FGameplayTag& ComparePhaseTag) const;
	
		FGameplayTag PhaseTag;
		EPhaseTagMatchType MatchType = EPhaseTagMatchType::ExactMatch;
		FCrimGamePhaseTagDelegate PhaseCallback;
	};

	TArray<FPhaseObserver> PhaseStartObservers;
	TArray<FPhaseObserver> PhaseEndObservers;

	friend class UGamePhaseGameplayAbility;
};
