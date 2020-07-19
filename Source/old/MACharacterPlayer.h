// ILikeBanas

#pragma once

#include "CoreMinimal.h"



/**
 * Extension of the CSS ChracterPlayer
 */
// UCLASS( config = Game )
class EXAMPLEMOD_API AFGCharacterPlayerExt
{
private:
    // GENERATED_BODY()

public:
    AFGCharacterPlayerExt();

	~AFGCharacterPlayerExt();

    /** @return The inventory component for the given equipment slot. */
  //  UFUNCTION(BlueprintPure, Category = "Equipment")
//    class UFGInventoryComponentEquipment* GetEquipmentSlot(EEquipmentSlot slot) const;

    // Register 
    // RegisterIncomingAttacker_Implementation( class AFGEnemyController* forController ) 
    // check if alive if not i
    // void UnregisterAttacker_Implementation( class AFGEnemyController* forController ) override;

private:
    //UPROPERTY(SaveGame, Replicated)
    unsigned long int experience = 0;

    //UPROPERTY(SaveGame, Replicated)
    unsigned long int level = 1;

    //UPROPERTY(SaveGame, Replicated)
    class UFGInventoryComponentEquipment* mFeetSlot;

    //UPROPERTY(SaveGame, Replicated)
    class UFGInventoryComponentEquipment* mHeadSlot;

    //UPROPERTY(SaveGame, Replicated)
    class UFGInventoryComponentEquipment* mChestSlot;

    //UPROPERTY(SaveGame, Replicated)
    class UFGInventoryComponentEquipment* mShoulderSlot;

    //UPROPERTY(SaveGame, Replicated)
    class UFGInventoryComponentEquipment* mWristSlot;
};


void registerCharacterHooks();
