// Fill out your copyright notice in the Description page of Project Settings.


#include "Tests/AutoMeshTest.h"

#include "AutoMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"
#include "UObject/ConstructorHelpers.h"


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FTestGetAssetMapActor,
	"Texturematica.AutoMesh.TestGetAssetMapActor",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FTestGetAssetMapActor::RunTest(const FString& Parameters)
{
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (IsValid(TestWorld))
	{
		AStaticMeshActor* CubeMeshActor = TestWorld->SpawnActor<AStaticMeshActor>();
		checkf(CubeMeshActor != nullptr, TEXT("nullptr: CubeMeshActor"));
		UStaticMeshComponent* CubeMeshComponent = NewObject<UStaticMeshComponent>(
			CubeMeshActor,
			UStaticMeshComponent::StaticClass(),
			NAME_None
			);
		checkf(CubeMeshComponent != nullptr, TEXT("nullptr: CubeMeshComponent"));
		CubeMeshComponent->RegisterComponent();
		UStaticMesh* CubeMesh = Cast<UStaticMesh>(
			StaticLoadObject(
				UStaticMesh::StaticClass(),
				CubeMeshComponent,
				TEXT("/Engine/BasicShapes/Cube.Cube")
			)
		);
		checkf(CubeMesh != nullptr, TEXT("nullptr: CubeMesh"));
		CubeMeshComponent->SetStaticMesh(CubeMesh);
		TMap<FString, FString> AssetMap = AAutoMesh::GetAssetMap(CubeMesh);
		TestEqual(TEXT("AssetMap ObjectName"), AssetMap["ObjectName"], TEXT("Cube"));
		TestEqual(TEXT("AssetMap ObjectPath"), AssetMap["ObjectPath"], TEXT("/Engine/BasicShapes/Cube.Cube"));
		TestEqual(TEXT("AssetMap PackageName"), AssetMap["PackageName"], TEXT("/Engine/BasicShapes/Cube"));
		TestEqual(TEXT("AssetMap PackagePath"), AssetMap["PackagePath"], TEXT("/Engine/BasicShapes"));
		TestWorld->DestroyWorld(false);
		return true;
	}
	else
	{
		return false;
	}
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FTestGetStaticMeshActor,
	"Texturematica.AutoMesh.TestGetStaticMeshActor",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FTestGetStaticMeshActor::RunTest(const FString& Parameters)
{
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (IsValid(TestWorld))
	{
		AStaticMeshActor* CubeMeshActor = TestWorld->SpawnActor<AStaticMeshActor>();
		checkf(CubeMeshActor != nullptr, TEXT("nullptr: CubeMeshActor"));
		UStaticMeshComponent* CubeMeshComponent = NewObject<UStaticMeshComponent>(
			CubeMeshActor,
			UStaticMeshComponent::StaticClass(),
			NAME_None
			);
		checkf(CubeMeshComponent != nullptr, TEXT("nullptr: CubeMeshComponent"));
		CubeMeshComponent->RegisterComponent();
		UStaticMesh* CubeMesh = Cast<UStaticMesh>(
			StaticLoadObject(
				UStaticMesh::StaticClass(),
				CubeMeshComponent,
				TEXT("/Engine/BasicShapes/Cube.Cube")
			)
		);
		checkf(CubeMesh != nullptr, TEXT("nullptr: CubeMesh"));
		CubeMeshComponent->SetStaticMesh(CubeMesh);
		TestEqual(TEXT("Testing Class"), CubeMesh->GetClass()->GetName(), TEXT("StaticMesh"));
		TestWorld->DestroyWorld(false);
		return true;
	}
	else
	{
		return false;
	}
}

BEGIN_DEFINE_SPEC(
	SpecGetStaticMesh,
	"Texturematica.AutoMesh.SpecGetStaticMesh",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)
	UWorld* TestWorld;
	UStaticMesh* CubeMesh;
	UStaticMeshComponent* CubeMeshComponent;
	AStaticMeshActor* CubeMeshActor;
END_DEFINE_SPEC(SpecGetStaticMesh)

void SpecGetStaticMesh::Define()
{
	Describe("Execute()", [this]()
	{
		BeforeEach([this]()
		{
			TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
			TestNotNull("TestWorld Exists", TestWorld);
			CubeMeshActor = TestWorld->SpawnActor<AStaticMeshActor>();
			TestNotNull("CubeMeshActor Exists", CubeMeshActor);
			UPackage* ComponentPackage = GetTransientPackage();
			CubeMeshComponent = NewObject<UStaticMeshComponent>(
				ComponentPackage,
				UStaticMeshComponent::StaticClass(),
				NAME_None
				);
			TestNotNull("CubeMeshComponent Exists", CubeMeshComponent);
			CubeMeshComponent->SetupAttachment(CubeMeshActor->GetRootComponent());
			CubeMeshComponent->RegisterComponent();
			CubeMesh = Cast<UStaticMesh>(
				StaticLoadObject(
					UStaticMesh::StaticClass(),
					CubeMeshComponent,
					TEXT("/Engine/BasicShapes/Cube.Cube")
				)
			);
			TestNotNull("CubeMesh Exists", CubeMesh);
			CubeMeshComponent->SetStaticMesh(CubeMesh);			
		});

		It("should return a valid StaticMesh from StaticMeshActor", 
			[this]()
		{
			UStaticMesh* SM = AAutoMesh::GetStaticMesh(CubeMeshActor);
			TestEqual(
				TEXT("Testing StaticMeshActor"),
				SM->GetClass()->GetName(),
				TEXT("StaticMesh")
			);	
		});
		
		It("should return a valid StaticMesh from StaticMeshComponent", 
			[this]()
		{
			UStaticMesh* SM = AAutoMesh::GetStaticMesh(CubeMeshComponent);
			TestEqual(TEXT("Testing StaticMeshComponent"),
				SM->GetClass()->GetName(),
				TEXT("StaticMesh")
			);	
		});
		
		It("should return a valid StaticMesh from StaticMesh", 
			[this]()
		{
			UStaticMesh* SM = AAutoMesh::GetStaticMesh(CubeMesh);
			TestEqual(TEXT("Testing StaticMesh"),
				SM->GetClass()->GetName(),
				TEXT("StaticMesh")
			);	
		});
		
		AfterEach([this]()
		{
			TestWorld = GEngine->GetWorldFromContextObject(CubeMeshActor, EGetWorldErrorMode::ReturnNull);
			TestNotNull("TestWorld Exists", TestWorld);
			TestWorld->DestroyWorld(false);
		});
	});
}
