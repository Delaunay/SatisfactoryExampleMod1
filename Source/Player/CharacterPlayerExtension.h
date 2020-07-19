// ILikeBanas

#pragma once

#include <functional>

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CharacterPlayerExtension.generated.h"


UENUM(BlueprintType)
enum class EEquipmentSlotExt :uint8 {
    ES_NONE			UMETA(DisplayName = "Please specify a slot."),
    ES_ARMS			UMETA(DisplayName = "Arms"),
    ES_BACK			UMETA(DisplayName = "Body"), // JetPack / Parachute

    // Extensions
    // Blade Runner
    ES_FEET			UMETA(DisplayName = "Feet"),
    // Gax Max
    ES_HEAD			UMETA(DisplayName = "Head"),
    // Hazmat Suit
    ES_CHEST        UMETA(DisplayName = "Chest"),

    // New: Nothing here
    // TBD: Auto-Defence Laser
    ES_SHOULDER     UMETA(DisplayName = "Shoulder"),

    // TBD: Morphin Injector
    ES_WRIST        UMETA(DisplayName = "Wrist"),

    ES_MAX          UMETA(Hidden)
};


/* Player Component that will attach itself to the original player component and add functionality
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class EXAMPLEMOD_API UCharacterPlayerExtension : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCharacterPlayerExtension();


    // Register a new PlayerExtension using Hooks
    static void Register();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
    /** @return The inventory component for the given equipment slot. */
    UFUNCTION(BlueprintPure, Category = "Equipment")
    class UFGInventoryComponentEquipment* GetEquipmentSlot(EEquipmentSlotExt slot) const;

    /** @return The active equipment in the given equipment slot; nullptr if nothing is equipped. */
    UFUNCTION(BlueprintPure, Category = "Equipment")
    class AFGEquipment* GetEquipmentInSlot(EEquipmentSlotExt slot) const;

    /** Must be called on the owning client for the client to be able to switch the weapon */
    UFUNCTION(BlueprintCallable, Category = "Equipment")
    void EquipEquipment(AFGEquipment* equipment);

    /** Must be called on the owning client to unequip one equipment */
    UFUNCTION(BlueprintCallable, Category = "Equipment")
    void UnequipEquipment(AFGEquipment* equipment);

    /*
    UFUNCTION(Reliable, Server, WithValidation)
    void Server_EquipEquipment(AFGEquipment* newEquipment);

    UFUNCTION(Reliable, Server, WithValidation)
    void Server_UnequipEquipment(AFGEquipment* newEquipment);
    */

    // Register 
    // RegisterIncomingAttacker_Implementation( class AFGEnemyController* forController ) 
    // check if alive if not i
    // void UnregisterAttacker_Implementation( class AFGEnemyController* forController ) override;

    // Experience System
    int experience;
    int level;

    // Extended Inventory
    class UFGInventoryComponentEquipment* mFeetSlot;
    class UFGInventoryComponentEquipment* mHeadSlot;
    class UFGInventoryComponentEquipment* mChestSlot;
    class UFGInventoryComponentEquipment* mShoulderSlot;
    class UFGInventoryComponentEquipment* mWristSlot;
};
