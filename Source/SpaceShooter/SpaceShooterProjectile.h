// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpaceShooterProjectile.generated.h"

UCLASS(Abstract, Blueprintable)
class SPACESHOOTER_API ASpaceShooterProjectile : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	float MaxSpeed;
	
public:	
	// Sets default values for this actor's properties
	ASpaceShooterProjectile(const FObjectInitializer& ObjectInitializer);
	/** initial setup */
	virtual void PostInitializeComponents() override;

	/** setup velocity */
	void InitVelocity(FVector& ShootDirection);

	/** handle hit */
	UFUNCTION()
	void OnImpact(const FHitResult& HitResult);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnExploded"))
	void OnExploded(const FVector &ImpactLocation, AActor *HitActor);

protected:

	/** controller that fired me (cache for damage calculations) */
	TWeakObjectPtr<AController> MyController;

	/** did it explode? */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_Exploded)
	bool bExploded;

	/** [client] explosion happened */
	UFUNCTION()
	void OnRep_Exploded();

	/** trigger explosion */
	void Explode(const FHitResult& Impact);

	/** shutdown projectile and prepare for destruction */
	void DisableAndDestroy();

	/** update velocity on client */
	virtual void PostNetReceiveVelocity(const FVector& NewVelocity) override;

private:
	/** movement component */
	UPROPERTY(VisibleDefaultsOnly, Category = "Projectile")
	class UProjectileMovementComponent *MovementComp;

	/** collisions */
	UPROPERTY(VisibleDefaultsOnly, Category = "Projectile")
	class USphereComponent* CollisionComp;

	UPROPERTY(VisibleDefaultsOnly, Category = "Projectile")
	class UParticleSystemComponent* ParticleComp;

protected:
	///** Returns MovementComp subobject **/
	//FORCEINLINE UProjectileMovementComponent* GetMovementComp() const { return MovementComp; }
	///** Returns CollisionComp subobject **/
	//FORCEINLINE USphereComponent* GetCollisionComp() const { return CollisionComp; }
	///** Returns ParticleComp subobject **/
	//FORCEINLINE UParticleSystemComponent* GetParticleComp() const { return ParticleComp; }
};
