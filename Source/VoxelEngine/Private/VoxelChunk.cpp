// Fill out your copyright notice in the Description page of Project Settings.


#include "VoxelChunk.h"
#include "VoxelWorld.h"
#include "DrawDebugHelpers.h"
#include <array>
#include "DynamicMesh/DynamicMesh3.h"
#include "DynamicMesh/MeshAttributeUtil.h"
#include "DynamicMesh/DynamicMeshAttributeSet.h"
#include "VoxelEngine/VoxelEngine.h"


void FVoxelChunkSecondaryTickFunction::ExecuteTick(float DeltaTime, ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent)
{
	if (!IsValid(Target) || !IsValid(Target->GetOwner()))
	{
		return;
	}
	if (Target->GetOwner()->IsPendingKillPending())
	{
		return;
	}
	if (!Target->GetOwner()->AllowReceiveTickEventOnDedicatedServer() && IsRunningDedicatedServer())
	{
		return;
	}
	if ((TickType == LEVELTICK_ViewportsOnly) && !Target->GetOwner()->ShouldTickIfViewportsOnly())
	{
		return;
	}

	FScopeCycleCounterUObject ActorScope(Target);
	Target->TickSecondary(DeltaTime * Target->GetOwner()->CustomTimeDilation, TickType, CurrentThread, MyCompletionGraphEvent, this);
}

FString FVoxelChunkSecondaryTickFunction::DiagnosticMessage()
{
	return FString();
}


// Sets default values for this component's properties
UVoxelChunk::UVoxelChunk()
{
	PrimaryComponentTick.bCanEverTick = true;
	SecondaryComponentTick.TickGroup = TG_LastDemotable;
	SecondaryComponentTick.bCanEverTick = true;
	SecondaryComponentTick.bStartWithTickEnabled = true;
	SecondaryComponentTick.TickInterval = 0.1;
}

// Called when the game starts
void UVoxelChunk::BeginPlay()
{
	Super::BeginPlay();

	AVoxelWorld* VoxelWorld = GetOwner<AVoxelWorld>();
	check(VoxelWorld);

	if (!IsTemplate() && SecondaryComponentTick.bCanEverTick)
	{
		SecondaryComponentTick.Target = this;
		SecondaryComponentTick.SetTickFunctionEnable(
			SecondaryComponentTick.bStartWithTickEnabled);
		SecondaryComponentTick.RegisterTickFunction(VoxelWorld->GetLevel());
	}

	FIntVector MinVoxel;
	FIntVector MaxVoxel;
	GetVoxelBoundingBox(MinVoxel, MaxVoxel);
	FVector Location = FVector(MinVoxel.X, MinVoxel.Y, MinVoxel.Z) * VoxelWorld->GetVoxelSizeWorld();
	SetRelativeLocation(Location);	

	DynamicMeshComponent = NewObject<UDynamicMeshComponent>(this);
	check(DynamicMeshComponent);
	DynamicMeshComponent->RegisterComponent();
	FAttachmentTransformRules Rules(EAttachmentRule::KeepRelative, false);
	DynamicMeshComponent->AttachToComponent(this, Rules);

	VisibleVoxelIndices.SetNum(VoxelWorld->GetChunkSide() * VoxelWorld->GetChunkSide() * VoxelWorld->GetWorldHeight(), false);
	GenerateMesh();
}

void UVoxelChunk::GenerateMesh()
{
	AVoxelWorld* VoxelWorld = GetOwner<AVoxelWorld>();
	check(VoxelWorld);

	checkSlow(DynamicMeshComponent);
	UDynamicMesh* MeshObj = DynamicMeshComponent->GetDynamicMesh();
	checkSlow(MeshObj);
	FDynamicMesh3& Mesh = MeshObj->GetMeshRef();
	VisibleVoxelIndices.Init(false, VisibleVoxelIndices.Num());
	Mesh.Clear();
	Mesh.EnableVertexUVs(FVector2f(0, 0));
	Mesh.EnableVertexColors(FVector3f(1, 1, 1));
	Mesh.EnableAttributes();
	Mesh.Attributes()->EnablePrimaryColors();

	ProcessVoxels();
	bool bIsValid = true;
	
	UE::Geometry::FDynamicMesh3::FValidityOptions ValidityOptions;
	UE::Geometry::EValidityCheckFailMode ValidityCheckFailMode = UE::Geometry::EValidityCheckFailMode::Ensure;
	bIsValid = Mesh.CheckValidity(ValidityOptions, ValidityCheckFailMode);
	if (bIsValid)
	{
		UE_LOG(LogTemp, Display, TEXT("Dynamic Mesh passed validity check"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Dynamic Mesh failed validity check"));
	}
	
	UE::Geometry::FDynamicMeshColorOverlay* ColorOverlay = Mesh.Attributes()->PrimaryColors();
	check(ColorOverlay);
	
	bIsValid = ColorOverlay->CheckValidity(false, ValidityCheckFailMode);
	if (bIsValid)
	{
		UE_LOG(LogTemp, Display, TEXT("Dynamic Mesh Color Overlay passed validity check"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Dynamic Mesh  Color Overlay failed validity check"));
		return;
	}
	
	bIsValid = UE::Geometry::CopyVertexUVsToOverlay(Mesh, *Mesh.Attributes()->PrimaryUV());
	check(bIsValid);
	bIsValid = CopyVertexColorsToOverlay(Mesh, *Mesh.Attributes()->PrimaryColors(), false);
	check(bIsValid);
	DynamicMeshComponent->SetMaterial(0, VoxelWorld->GetVoxelChunkMaterial());
	DynamicMeshComponent->NotifyMeshUpdated();
	DynamicMeshComponent->SetEnableFlatShading(true);
	bIsMeshDirty = false;
}

void UVoxelChunk::ProcessVoxels()
{
	AVoxelWorld* VoxelWorld = GetOwner<AVoxelWorld>();
	check(VoxelWorld);

	int32 ChunkSide = VoxelWorld->GetChunkSide();
	int32 WorldHeightVoxel = VoxelWorld->GetWorldHeight();

	for (int X = 0; X < ChunkSide; X++)
	{
		for (int Y = 0; Y < ChunkSide; Y++)
		{
			for (int Z = 0; Z < WorldHeightVoxel; Z++)
			{
				ProcessVoxel(X, Y, Z);
			}
		}
	}
}

void UVoxelChunk::ProcessVoxel(int32 X, int32 Y, int32 Z)
{
	AVoxelWorld* VoxelWorld = GetOwner<AVoxelWorld>();
	checkSlow(VoxelWorld);

	FIntVector ChunkMin;
	FIntVector ChunkMax;
	GetVoxelBoundingBox(ChunkMin, ChunkMax);

	FIntVector CoordTranslated = ChunkMin + FIntVector(X, Y, Z);
	checkSlow(CoordTranslated.X <= ChunkMax.X);
	checkSlow(CoordTranslated.Y <= ChunkMax.Y);
	checkSlow(CoordTranslated.Z <= ChunkMax.Z);

	if (VoxelWorld->IsVoxelTransparent(CoordTranslated))
	{
		return;
	}

	const Voxel& Voxel = VoxelWorld->GetVoxel(CoordTranslated);

	VoxelWorld->IsVoxelTransparent(CoordTranslated);

	TStaticArray<bool, 6> FacesVisibility;
	int VisibleFacesNum = CheckVoxelSidesVisibility(CoordTranslated, FacesVisibility);
	for (int I = 0; I < FacesVisibility.Num(); I++)
	{
		if (FacesVisibility[I])
		{
			AddFaceData(Voxel, X, Y, Z, I);
			VisibleFacesNum++;
		}
	}
	if (VisibleFacesNum > 0)
	{
		int32 VoxelIndex = LinearizeCoordinate(X, Y, Z);
		VisibleVoxelIndices[VoxelIndex] = true;
	}
}

bool UVoxelChunk::IsFaceVisible(int32 X, int32 Y, int32 Z) const
{
	AVoxelWorld* VoxelWorld = GetOwner<AVoxelWorld>();
	checkSlow(VoxelWorld);

	return VoxelWorld->IsVoxelTransparent(FIntVector(X, Y, Z));
}

void UVoxelChunk::AddFaceData(const Voxel& Voxel, int32 X, int32 Y, int32 Z, int FaceIndex)
{
	AVoxelWorld* VoxelWorld = GetOwner<AVoxelWorld>();
	check(VoxelWorld);
	double VoxelSizeWorld = VoxelWorld->GetVoxelSizeWorld();
	check(DynamicMeshComponent);
	UDynamicMesh* MeshObject = DynamicMeshComponent->GetDynamicMesh();
	check(MeshObject);
	FDynamicMesh3& Mesh = MeshObject->GetMeshRef();
	auto* UvLayer = Mesh.Attributes()->GetUVLayer(0);
	check(UvLayer);
	UE::Geometry::FDynamicMeshColorOverlay* ColorOverlay = Mesh.Attributes()->PrimaryColors();
	check(ColorOverlay);

	int32 VoxelTypeInt = Voxel.VoxelTypeId - 1;
	int32 VoxelTypeNum = VoxelWorld->GetVoxelTypeSet()->GetVoxelTypes().Num();
	float UMin = static_cast<float>(FaceIndex) / 6;
	float UMax = static_cast<float>(FaceIndex + 1) / 6;
	float VMin = static_cast<float>(VoxelTypeInt) / VoxelTypeNum;
	float VMax = static_cast<float>(VoxelTypeInt + 1) / VoxelTypeNum;

	TStaticArray<UE::Geometry::FVertexInfo, 4> VertexInfos;
	for (int I = 0; I < VertexInfos.Num(); I++)
	{
		VertexInfos[I].bHaveC = true;
		VertexInfos[I].bHaveUV = true;
	}

	if (FaceIndex == 0) // Top
	{
		VertexInfos[0].Position = FVector(X, Y, Z + 1) * VoxelSizeWorld;
		VertexInfos[1].Position = FVector(X + 1, Y, Z + 1) * VoxelSizeWorld;
		VertexInfos[2].Position = FVector(X + 1, Y + 1, Z + 1) * VoxelSizeWorld;
		VertexInfos[3].Position = FVector(X, Y + 1, Z + 1) * VoxelSizeWorld;

		VertexInfos[0].UV = FVector2f(UMin, VMax);
		VertexInfos[1].UV = FVector2f(UMin, VMin);
		VertexInfos[2].UV = FVector2f(UMax, VMin);
		VertexInfos[3].UV = FVector2f(UMax, VMax);
	}
	else if (FaceIndex == 1) // Bottom
	{
		VertexInfos[0].Position = FVector(X + 1, Y, Z) * VoxelSizeWorld;
		VertexInfos[1].Position = FVector(X, Y, Z) * VoxelSizeWorld;
		VertexInfos[2].Position = FVector(X, Y + 1, Z) * VoxelSizeWorld;
		VertexInfos[3].Position = FVector(X + 1, Y + 1, Z) * VoxelSizeWorld;

		VertexInfos[0].UV = FVector2f(UMin, VMax);
		VertexInfos[1].UV = FVector2f(UMin, VMin);
		VertexInfos[2].UV = FVector2f(UMax, VMin);
		VertexInfos[3].UV = FVector2f(UMax, VMax);
	}
	else if (FaceIndex == 2) // Front
	{
		VertexInfos[0].Position = FVector(X + 1, Y, Z + 1) * VoxelSizeWorld;
		VertexInfos[1].Position = FVector(X + 1, Y, Z) * VoxelSizeWorld;
		VertexInfos[2].Position = FVector(X + 1, Y + 1, Z) * VoxelSizeWorld;
		VertexInfos[3].Position = FVector(X + 1, Y + 1, Z + 1) * VoxelSizeWorld;

		VertexInfos[0].UV = FVector2f(UMin, VMin);
		VertexInfos[1].UV = FVector2f(UMin, VMax);
		VertexInfos[2].UV = FVector2f(UMax, VMax);
		VertexInfos[3].UV = FVector2f(UMax, VMin);
	}
	else if (FaceIndex == 3) // Back
	{
		VertexInfos[0].Position = FVector(X, Y, Z) * VoxelSizeWorld;
		VertexInfos[1].Position = FVector(X, Y, Z + 1) * VoxelSizeWorld;
		VertexInfos[2].Position = FVector(X, Y + 1, Z + 1) * VoxelSizeWorld;
		VertexInfos[3].Position = FVector(X, Y + 1, Z) * VoxelSizeWorld;

		VertexInfos[0].UV = FVector2f(UMin, VMax);
		VertexInfos[1].UV = FVector2f(UMin, VMin);
		VertexInfos[2].UV = FVector2f(UMax, VMin);
		VertexInfos[3].UV = FVector2f(UMax, VMax);
	}
	else if (FaceIndex == 4) // Left
	{
		VertexInfos[0].Position = FVector(X, Y, Z) * VoxelSizeWorld;
		VertexInfos[1].Position = FVector(X + 1, Y, Z) * VoxelSizeWorld;
		VertexInfos[2].Position = FVector(X + 1, Y, Z + 1) * VoxelSizeWorld;
		VertexInfos[3].Position = FVector(X, Y, Z + 1) * VoxelSizeWorld;

		VertexInfos[0].UV = FVector2f(UMax, VMax);
		VertexInfos[1].UV = FVector2f(UMin, VMax);
		VertexInfos[2].UV = FVector2f(UMin, VMin);
		VertexInfos[3].UV = FVector2f(UMax, VMin);
	}
	else if (FaceIndex == 5) // Right
	{
		VertexInfos[0].Position = FVector(X,		Y + 1, Z	) * VoxelSizeWorld;
		VertexInfos[1].Position = FVector(X,		Y + 1, Z + 1) * VoxelSizeWorld;
		VertexInfos[2].Position = FVector(X + 1, Y + 1, Z + 1) * VoxelSizeWorld;
		VertexInfos[3].Position = FVector(X + 1, Y + 1, Z	) * VoxelSizeWorld;

		VertexInfos[0].UV = FVector2f(UMin, VMax);
		VertexInfos[1].UV = FVector2f(UMin, VMin);
		VertexInfos[2].UV = FVector2f(UMax, VMin);
		VertexInfos[3].UV = FVector2f(UMax, VMax);
	}

	int32 VertexIdMin = Mesh.MaxVertexID();
	int32 TriangleIdMin = Mesh.MaxTriangleID();

	Mesh.BeginUnsafeVerticesInsert();
	for (int I = 0; I < VertexInfos.Num(); I++)
	{
		UE::Geometry::EMeshResult InsertResult = Mesh.InsertVertex(VertexIdMin + I, VertexInfos[I], true);
		if (InsertResult != UE::Geometry::EMeshResult::Ok)
		{
			UE_LOG(LogVoxelEngine, Error, TEXT("Failed to insert vertex %s to index %d: error code %d"), *VertexInfos[I].Position.ToString(), VertexIdMin + I, static_cast<int32>(InsertResult));
		}
	}
	Mesh.EndUnsafeVerticesInsert();

	UE::Geometry::FIndex3i Tri1(VertexIdMin + 2, VertexIdMin + 1, VertexIdMin);
	UE::Geometry::FIndex3i Tri2(VertexIdMin + 3, VertexIdMin + 2, VertexIdMin);
	Mesh.BeginUnsafeTrianglesInsert();
	UE::Geometry::EMeshResult TriInsertResult = Mesh.InsertTriangle(TriangleIdMin, Tri1, 0, true);
	if (TriInsertResult != UE::Geometry::EMeshResult::Ok)
	{
		UE_LOG(LogVoxelEngine, Error, TEXT("Failed to insert triangle (%d, %d, %d) to index %d: error code %d"), Tri1.A, Tri1.B, Tri1.C, TriangleIdMin, static_cast<int32>(TriInsertResult));
	}
	TriInsertResult = Mesh.InsertTriangle(TriangleIdMin + 1, Tri2, 0, true);
	if (TriInsertResult != UE::Geometry::EMeshResult::Ok)
	{
		UE_LOG(LogVoxelEngine, Error, TEXT("Failed to insert triangle (%d, %d, %d) to index %d: error code %d"), Tri2.A, Tri2.B, Tri2.C, TriangleIdMin + 1, static_cast<int32>(TriInsertResult));
	}
	Mesh.EndUnsafeTrianglesInsert();

	// TODO Remove this Grass hack
	if (FaceIndex == 0 && Voxel.VoxelTypeId == 1)
	{
		FVector3f VertexColor{ 0.4, 1, 0.4 };
		Mesh.SetVertexColor(VertexIdMin, VertexColor);
		Mesh.SetVertexColor(VertexIdMin + 1, VertexColor);
		Mesh.SetVertexColor(VertexIdMin + 2, VertexColor);
		Mesh.SetVertexColor(VertexIdMin + 3, VertexColor);
	}
	else
	{
		FVector3f VertexColor{ 1, 1, 1 };
		Mesh.SetVertexColor(VertexIdMin, VertexColor);
		Mesh.SetVertexColor(VertexIdMin + 1, VertexColor);
		Mesh.SetVertexColor(VertexIdMin + 2, VertexColor);
		Mesh.SetVertexColor(VertexIdMin + 3, VertexColor);
	}
}

int32 UVoxelChunk::LinearizeCoordinate(int32 X, int32 Y, int32 Z) const
{
	AVoxelWorld* VoxelWorld = GetOwner<AVoxelWorld>();
	check(VoxelWorld);
	int32 ChunkSide = VoxelWorld->GetChunkSide();
	int32 Result =
		static_cast<int32>(Z * ChunkSide * ChunkSide) +
		static_cast<int32>(Y * ChunkSide) +
		static_cast<int32>(X);
	return Result;
}

FIntVector UVoxelChunk::DelinearizeCoordinate(int32 LinearCoord) const
{
	AVoxelWorld* VoxelWorld = GetOwner<AVoxelWorld>();
	check(VoxelWorld);
	int32 ChunkSide = VoxelWorld->GetChunkSide();
	int32 X = LinearCoord % (ChunkSide);
	int32 Y = (LinearCoord / (ChunkSide)) % (ChunkSide);
	int32 Z = LinearCoord / ((ChunkSide) * (ChunkSide));
	return FIntVector(X, Y, Z);
}

void UVoxelChunk::ProcessChangeRequest(const FVoxelChange& Request)
{
	AVoxelWorld* VoxelWorld = GetOwner<AVoxelWorld>();
	check(VoxelWorld);

	if (Request.ChangeToVoxelType == Request.ExpectedVoxelType)
	{
		return;
	}

	FIntVector ChunkMin;
	FIntVector ChunkMax;
	GetVoxelBoundingBox(ChunkMin, ChunkMax);

	FIntVector LocalCoord = Request.Coordinate - ChunkMin;
	int32 VoxelIndex = LinearizeCoordinate(LocalCoord.X, LocalCoord.Y, LocalCoord.Z);
	bool bWasVisible = VisibleVoxelIndices[VoxelIndex];
	bool bIsNowVisible = VoxelWorld->IsVoxelTransparent(Request.Coordinate);
	if (bWasVisible == bIsNowVisible)
	{
		return;
	}

	UpdateVoxelVisibility(Request.Coordinate, true);

	VisibleVoxelIndices[VoxelIndex] = bIsNowVisible;
	
	bIsMeshDirty = true;
}


bool UVoxelChunk::CopyVertexColorsToOverlay(const FDynamicMesh3& Mesh, UE::Geometry::FDynamicMeshColorOverlay& ColorOverlayOut, bool bCompactElements)
{
	if (!Mesh.HasVertexColors())
	{
		return false;
	}

	if (ColorOverlayOut.ElementCount() > 0)
	{
		ColorOverlayOut.ClearElements();
	}

	ColorOverlayOut.BeginUnsafeElementsInsert();
	for (int32 Vid : Mesh.VertexIndicesItr())
	{
		auto Color = Mesh.GetVertexColor(Vid);
		std::array<float, 4> ColorArray{ Color.X, Color.Y, Color.Z, 1};
		ColorOverlayOut.InsertElement(Vid, ColorArray.data(), true);
	}
	ColorOverlayOut.EndUnsafeElementsInsert();

	for (int32 Tid : Mesh.TriangleIndicesItr())
	{
		ColorOverlayOut.SetTriangle(Tid, Mesh.GetTriangle(Tid));
	}

	if (bCompactElements)
	{
		UE::Geometry::FCompactMaps CompactMaps;
		ColorOverlayOut.CompactInPlace(CompactMaps);
	}

	return true;
}

int UVoxelChunk::CheckVoxelSidesVisibility(const FIntVector& CoordTranslated, TStaticArray<bool, 6>& SideVisilityFlags)
{
	SideVisilityFlags[0] = IsFaceVisible(CoordTranslated.X, CoordTranslated.Y, CoordTranslated.Z + 1); // Top
	SideVisilityFlags[1] = IsFaceVisible(CoordTranslated.X, CoordTranslated.Y, CoordTranslated.Z - 1); // Bottom
	SideVisilityFlags[2] = IsFaceVisible(CoordTranslated.X + 1, CoordTranslated.Y, CoordTranslated.Z); // Front
	SideVisilityFlags[3] = IsFaceVisible(CoordTranslated.X - 1, CoordTranslated.Y, CoordTranslated.Z); // Back
	SideVisilityFlags[4] = IsFaceVisible(CoordTranslated.X, CoordTranslated.Y - 1, CoordTranslated.Z); // Left
	SideVisilityFlags[5] = IsFaceVisible(CoordTranslated.X, CoordTranslated.Y + 1, CoordTranslated.Z); // Right
	int VisibleFacesNum = 0;
	for (int I = 0; I < SideVisilityFlags.Num(); I++)
	{
		if (SideVisilityFlags[I])
		{
			VisibleFacesNum++;
		}
	}

	return VisibleFacesNum;
}

void UVoxelChunk::UpdateVoxelVisibility(const FIntVector& VoxelWorldCoord, bool bUpdateNeighbours)
{
	AVoxelWorld* VoxelWorld = GetOwner<AVoxelWorld>();
	check(VoxelWorld);
	UVoxelChunk* VoxelOwner = VoxelWorld->GetChunkFromVoxelCoord(VoxelWorldCoord);
	if (VoxelOwner == this)
	{
		FIntVector ChunkMin;
		FIntVector ChunkMax;
		GetVoxelBoundingBox(ChunkMin, ChunkMax);

		FIntVector LocalCoord = VoxelWorldCoord - ChunkMin;
		int32 Index = LinearizeCoordinate(LocalCoord.X, LocalCoord.Y, LocalCoord.Z);
		TStaticArray<bool, 6> SidesFlags;
		int VisibleFacesNum = CheckVoxelSidesVisibility(VoxelWorldCoord, SidesFlags);
		bool OldVisibility = VisibleVoxelIndices[Index];
		bool NewVisibility = VisibleFacesNum > 0;
		if (OldVisibility == NewVisibility)
		{
			return;
		}
		
		VisibleVoxelIndices[Index] = NewVisibility;

		if (!bUpdateNeighbours)
		{
			return;
		}

		TStaticArray<FIntVector, 6> NeighbourVoxelWorldCoords
		{
			FIntVector(VoxelWorldCoord.X, VoxelWorldCoord.Y, VoxelWorldCoord.Z + 1),
			FIntVector(VoxelWorldCoord.X, VoxelWorldCoord.Y, VoxelWorldCoord.Z - 1),
			FIntVector(VoxelWorldCoord.X + 1, VoxelWorldCoord.Y, VoxelWorldCoord.Z),
			FIntVector(VoxelWorldCoord.X - 1, VoxelWorldCoord.Y, VoxelWorldCoord.Z),
			FIntVector(VoxelWorldCoord.X, VoxelWorldCoord.Y + 1, VoxelWorldCoord.Z),
			FIntVector(VoxelWorldCoord.X, VoxelWorldCoord.Y - 1, VoxelWorldCoord.Z)
		};
		
		for (const FIntVector& Coord : NeighbourVoxelWorldCoords)
		{
			UpdateVoxelVisibility(Coord, false);
		}
	}
	else
	{
		VoxelOwner->UpdateVoxelVisibility(VoxelWorldCoord, bUpdateNeighbours);
	}
}


// Called every frame
void UVoxelChunk::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bDebugDrawDimensions)
	{
		AVoxelWorld* VoxelWorld = GetOwner<AVoxelWorld>();
		checkSlow(VoxelWorld);
		FBox3d Bbox = GetWorldBoundingBox();
		DrawDebugBox(GetWorld(), Bbox.GetCenter(), Bbox.GetExtent(), FColor::Green);
	}

	FVoxelChange ChangeRequest;
	while (VoxelChangeRequests.Dequeue(ChangeRequest))
	{
		ProcessChangeRequest(ChangeRequest);
	}
}

void UVoxelChunk::TickSecondary(float DeltaTime, ELevelTick LevelTick, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent, FVoxelChunkSecondaryTickFunction* TickFunction)
{
	if (bIsMeshDirty)
	{
		FDateTime StartTime = FDateTime::Now();
		RegenerateMesh();
		FDateTime EndTime = FDateTime::Now();
		FTimespan ElapsedTime = EndTime - StartTime;
		UE_LOG(LogVoxelEngine, Display, TEXT("Chunk (%d, %d) mesh regenerated in %3.2f milliseconds"), ChunkX, ChunkY, ElapsedTime.GetTotalMilliseconds());
	}
}

int32 UVoxelChunk::GetChunkSide() const
{
	AVoxelWorld* VoxelWorld = GetOwner<AVoxelWorld>();
	checkSlow(VoxelWorld);
	return VoxelWorld->GetChunkSide();
}

void UVoxelChunk::GetChunkIndex(int32& OutX, int32& OutY) const
{
	OutX = ChunkX;
	OutY = OutY;
}

void UVoxelChunk::SetChunkIndex(int32 X, int32 Y)
{
	ChunkX = X;
	ChunkY = Y;
}

FBox UVoxelChunk::GetWorldBoundingBox() const
{
	AVoxelWorld* VoxelWorld = GetOwner<AVoxelWorld>();
	checkSlow(VoxelWorld);

	double VoxelSizeWorld = VoxelWorld->GetVoxelSizeWorld();

	FIntVector MinVoxel;
	FIntVector MaxVoxel;
	GetVoxelBoundingBox(MinVoxel, MaxVoxel);

	FVector Min = VoxelWorld->GetActorLocation() + FVector(MinVoxel.X, MinVoxel.Y, MinVoxel.Z) * VoxelSizeWorld;
	FVector Max = VoxelWorld->GetActorLocation() + FVector(MaxVoxel.X, MaxVoxel.Y, MaxVoxel.Z) * VoxelSizeWorld;
	return FBox3d(Min, Max);
}

void UVoxelChunk::GetVoxelBoundingBox(FIntVector& OutMin, FIntVector& OutMax) const
{
	AVoxelWorld* VoxelWorld = GetOwner<AVoxelWorld>();
	checkSlow(VoxelWorld);

	int32 ChunkSide = VoxelWorld->GetChunkSide();
	int32 WorldHeightVoxel = VoxelWorld->GetWorldHeight();
	int32 FirstBlockX = ChunkX * ChunkSide;
	int32 FirstBlockY = ChunkY * ChunkSide;
	int32 FirstBlockZ = 0;
	
	OutMin = FIntVector(FirstBlockX, FirstBlockY, FirstBlockZ);
	OutMax = FIntVector(FirstBlockX + ChunkSide, FirstBlockY + ChunkSide, FirstBlockZ + WorldHeightVoxel);
}

void UVoxelChunk::SetDrawWireframe(bool bEnabled)
{
	check(DynamicMeshComponent);
	DynamicMeshComponent->SetEnableWireframeRenderPass(bEnabled);
	bDebugDrawDimensions = bEnabled;
}

bool UVoxelChunk::GetDrawWireframe() const
{
	return bDebugDrawDimensions;
}

void UVoxelChunk::MarkMeshDirty()
{
	bIsMeshDirty = true;
}

EVoxelChangeResult UVoxelChunk::ChangeVoxelRendering(const FVoxelChange& VoxelChange)
{
	VoxelChangeRequests.Enqueue(VoxelChange);
	return EVoxelChangeResult::Executed;
}

void UVoxelChunk::RegenerateMesh()
{
	AVoxelWorld* VoxelWorld = GetOwner<AVoxelWorld>();
	check(VoxelWorld);

	checkSlow(DynamicMeshComponent);
	UDynamicMesh* MeshObj = DynamicMeshComponent->GetDynamicMesh();
	checkSlow(MeshObj);
	FDynamicMesh3& Mesh = MeshObj->GetMeshRef();

	Mesh.Clear();
	Mesh.EnableVertexUVs(FVector2f(0, 0));
	Mesh.EnableVertexColors(FVector3f(1, 1, 1));
	Mesh.EnableAttributes();
	Mesh.Attributes()->EnablePrimaryColors();

	for (TConstSetBitIterator<> It(VisibleVoxelIndices); It; ++It) 
	{
		FIntVector VoxelLocalCoord = DelinearizeCoordinate(It.GetIndex());
		ProcessVoxel(VoxelLocalCoord.X, VoxelLocalCoord.Y, VoxelLocalCoord.Z);
	}
	
	bool bIsValid = UE::Geometry::CopyVertexUVsToOverlay(Mesh, *Mesh.Attributes()->PrimaryUV());
	check(bIsValid);
	bIsValid = CopyVertexColorsToOverlay(Mesh, *Mesh.Attributes()->PrimaryColors(), false);
	check(bIsValid);
	DynamicMeshComponent->NotifyMeshUpdated();
	bIsMeshDirty = false;
}

