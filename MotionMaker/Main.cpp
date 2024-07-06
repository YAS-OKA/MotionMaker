# include <Siv3D.hpp> // Siv3D v0.6.14
#include"MySystem/GameMaster.h"
#include"Scenes/MotionCreator.h"

void Main()
{
	Window::Resize(1200, 675);
	System::SetTerminationTriggers(UserAction::CloseButtonClicked);

	EntityManager manager;

	auto gameMaster = manager.birth<GameMaster>();

	gameMaster->addScene(new mot::PartsEditor);	

	while (System::Update())
	{
		ClearPrint();

		// 1 秒間に何回メインループが実行されているかを取得する
		int32 fps = Profiler::FPS();

		// 1 秒間に何回メインループが実行されているかを、ウィンドウタイトルに表示する
		Window::SetTitle(fps);

		manager.update(Scene::DeltaTime());
	}
}
