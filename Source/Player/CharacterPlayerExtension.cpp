// ILikeBanas

#include "CharacterPlayerExtension.h"

#include "FGCharacterPlayer.h"
#include "FGInventoryComponentEquipment.h"
#include "FGEquipment.h"
#include "FGJetPack.h"

#include "util/Logging.h"
#include "mod/hooking.h"

#include "ExampleMod/AdvancedHooks.h"


/*
Equip_StunSpear_C
Equip_NobeliskDetonator_C
Equip_JetPack_C
Equip_ObjectScanner_C
Equip_Beacon_C
Equip_Chainsaw_C
Equip_MedKit_C

Equip_JumpingStilts_C
Equip_GasMask_C
Equip_HazmatSuit_C
*/

// Override the SlotEnum for some vanilla items
TMap<FString, EEquipmentSlotExt> MakeEquipmentSlotMapping() {
    TMap<FString, EEquipmentSlotExt> mapping;
    mapping.Add("Equip_JumpingStilts_C", EEquipmentSlotExt::ES_FEET);
    mapping.Add("Equip_GasMask_C"      , EEquipmentSlotExt::ES_HEAD);
    mapping.Add("Equip_HazmatSuit_C"   , EEquipmentSlotExt::ES_CHEST);
    return mapping;
}

TMap<FString, EEquipmentSlotExt> const& EquipmentSlotMapping() {
    static TMap<FString, EEquipmentSlotExt> mapping = MakeEquipmentSlotMapping();
    return mapping;
}

EEquipmentSlotExt GetEquipmentSlotEnumOverride(AFGEquipment* equipment) {
    auto name = equipment->GetClass()->GetName();

    if (EquipmentSlotMapping().Contains(name)) {
        return EquipmentSlotMapping()[name];
    }

    return static_cast<EEquipmentSlotExt>(AFGEquipment::GetEquipmentSlot(equipment->GetClass()));
}


void UCharacterPlayerExtension::Register() {
    SML::Logging::debug(TEXT("Registering UCharacterPlayerExtension hooks"));

    SUBSCRIBE_METHOD(AFGCharacterPlayer::BeginPlay, [](auto& Scope, AFGCharacterPlayer* Player) {
        UCharacterPlayerExtension* Component = NewObject<UCharacterPlayerExtension>(Player, TEXT("RPG_PlayerComponent"));

        SML::Logging::debug(TEXT("Setting RPG Extension to player"), *Player->GetPathName());

        Component->RegisterComponent();
        Component->SetNetAddressable();
        Component->SetIsReplicated(true);
    });

    SUBSCRIBE_METHOD(AFGCharacterPlayer::EquipEquipment, [](auto&, AFGCharacterPlayer* self, AFGEquipment* equipment) {
        SML::Logging::debug(TEXT("EquipEquipment:"), 
            *equipment->GetClass()->GetName());
    });

    /*
    CUST_SUBSCRIBE_METHOD_AFTER(AFGCharacterPlayer::GetActiveEquipments, [](AFGCharacterPlayer const* self, TArray<AFGEquipment*>& equipments) {
        
    });*/
}

// Sets default values for this component's properties
UCharacterPlayerExtension::UCharacterPlayerExtension()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

    level = 1;
    experience = 0;

    mFeetSlot     = NewObject<UFGInventoryComponentEquipment>(this, TEXT("InvFeetSlot"));
    mHeadSlot     = NewObject<UFGInventoryComponentEquipment>(this, TEXT("InvHeadSlot"));
    mChestSlot    = NewObject<UFGInventoryComponentEquipment>(this, TEXT("InvChestSlot"));
    mShoulderSlot = NewObject<UFGInventoryComponentEquipment>(this, TEXT("InvShoulderSlot"));
    mWristSlot    = NewObject<UFGInventoryComponentEquipment>(this, TEXT("InvWristSlot"));

    mFeetSlot    ->SetEquipmentSlotEnum(static_cast<EEquipmentSlot>(EEquipmentSlotExt::ES_FEET));
    mHeadSlot    ->SetEquipmentSlotEnum(static_cast<EEquipmentSlot>(EEquipmentSlotExt::ES_HEAD));
    mChestSlot   ->SetEquipmentSlotEnum(static_cast<EEquipmentSlot>(EEquipmentSlotExt::ES_CHEST));
    mShoulderSlot->SetEquipmentSlotEnum(static_cast<EEquipmentSlot>(EEquipmentSlotExt::ES_SHOULDER));
    mWristSlot   ->SetEquipmentSlotEnum(static_cast<EEquipmentSlot>(EEquipmentSlotExt::ES_WRIST));
}





// Called when the game starts
void UCharacterPlayerExtension::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


UFGInventoryComponentEquipment* UCharacterPlayerExtension::GetEquipmentSlot(EEquipmentSlotExt slot) const {
    switch (slot) {
    case EEquipmentSlotExt::ES_NONE:
    case EEquipmentSlotExt::ES_ARMS:
    case EEquipmentSlotExt::ES_BACK:
        return static_cast<AFGCharacterPlayer*>(this->GetOwner())->GetEquipmentSlot(static_cast<EEquipmentSlot>(slot));

    case EEquipmentSlotExt::ES_FEET:
        return mFeetSlot;
    case EEquipmentSlotExt::ES_HEAD:
        return mHeadSlot;
    case EEquipmentSlotExt::ES_CHEST:
        return mChestSlot;
    case EEquipmentSlotExt::ES_SHOULDER:
        return mShoulderSlot;
    case EEquipmentSlotExt::ES_WRIST:
        return mWristSlot;
    }

    return nullptr;
}

AFGEquipment* UCharacterPlayerExtension::GetEquipmentInSlot(EEquipmentSlotExt slot) const {
    auto inv = GetEquipmentSlot(slot);
    if (inv != nullptr) {
        return inv->GetEquipmentInSlot();
    }
    return nullptr;
}

void UCharacterPlayerExtension::EquipEquipment(AFGEquipment* equipment) {
    std::function<void()> original;

    auto slot = GetEquipmentSlotEnumOverride(equipment);
    auto player = static_cast<AFGCharacterPlayer*>(this->GetOwner());

    switch (slot) {
    case EEquipmentSlotExt::ES_NONE:
    case EEquipmentSlotExt::ES_ARMS:
    case EEquipmentSlotExt::ES_BACK:
        return original();

    default:
        auto inv = GetEquipmentSlot(slot);

        if (inv != nullptr){
            auto old = inv->GetEquipmentInSlot();

            if (old != nullptr){
                player->UnequipEquipment(old);
            }

            original();
        }
    }
}

void UCharacterPlayerExtension::UnequipEquipment(AFGEquipment* equipment) {
    auto slot = static_cast<EEquipmentSlotExt>(AFGEquipment::GetEquipmentSlot(equipment->GetClass()));
    auto player = static_cast<AFGCharacterPlayer*>(this->GetOwner());

    

    switch (slot) {
    case EEquipmentSlotExt::ES_NONE:
    case EEquipmentSlotExt::ES_ARMS:
    case EEquipmentSlotExt::ES_BACK:
        return player->UnequipEquipment(equipment);

    default:
        auto inv = GetEquipmentSlot(slot);

        if (inv != nullptr) {
            auto old = inv->GetEquipmentInSlot();

            if (old != nullptr) {
                player->UnequipEquipment(old);
            }
        }
    }
}