# include <Siv3D.hpp>

enum class Cell
{
	Block,
	Empty,
	Selected,
	Start,
	Goal,
	Route,
	Finded,
};

/// @brief ランダムな点を生成
/// @return Cell::Startの座標
Point RandomFill(Grid<Cell> &grid)
{
	for (auto y : Range(1, grid.height()-2))
	{
		for (auto x : Range(1, grid.width()-2))
		{
			grid[y][x] = RandomBool(0.35) ? Cell::Block : Cell::Empty;
		}
	}

	Point st = Point(Random(1ull, grid.width() - 2), Random(1ull, grid.height() - 2));
	Point gl = Point(Random(1ull, grid.width() - 2), Random(1ull, grid.height() - 2));
	grid[st] = Cell::Start;
	grid[gl] = Cell::Goal;

	return st;
}

/// @brief 穴掘り法を用いた迷路生成
/// @return Cell::Startの座標
Point MakeMaze(Grid<Cell>& grid)
{
	// 迷路の初期化
	grid.fill(Cell::Block);

	// 穴掘り法で迷路を生成
	Point st = Point(Random(1ull, grid.width() - 2), Random(1ull, grid.height() - 2));
	grid[st] = Cell::Start;
	Point gl = Point(Random(1ull, grid.width() - 2), Random(1ull, grid.height() - 2));
	grid[gl] = Cell::Goal;

	Array<Point> d = { {1,0},{0,1},{-1,0},{0,-1} };

	// ラムダ式による再帰
	std::function<void(Point)> dig = [&](Point now) -> void
	{
		grid[now] = Cell::Empty;
		d.shuffle();
		for (int i : step(4))
		{
			Point next = now + d[i];
			Point n_next = next + d[i];

			if (! (0 < n_next.x && n_next.x < grid.width() - 1 && 0 < n_next.y && n_next.y < grid.height() - 1) )
			{
				return;
			}

			if (grid[next] == Cell::Block && grid[n_next] == Cell::Block)
			{
				grid[next] = Cell::Empty;
				dig(n_next);
			}
		}
	};

	dig(st);
	grid[st] = Cell::Start;
	return st;
}

// フィールドの状態を画像化する関数
void CopyToImage(const Grid<Cell>& grid, Image& image)
{
	for (auto y : step(image.height()))
	{
		for (auto x : step(image.width()))
		{
			Cell cell = grid[y + 1][x + 1];
			if (cell == Cell::Block) image[y][x] = Palette::Green;
			if (cell == Cell::Empty || cell == Cell::Finded) image[y][x] = Palette::Black;
			if (cell == Cell::Selected) image[y][x] = Palette::Orange;
			if (cell == Cell::Start) image[y][x] = Palette::Red;
			if (cell == Cell::Goal) image[y][x] = Palette::Blue;
			if (cell == Cell::Route) image[y][x] = Palette::Floralwhite;
		}
	}
}

/// @brief 深さ優先探索で迷路を解く
/// @param grid : 迷路のマップ
/// @param stack : dfsで探索するためのスタック
/// @param route : スタートからゴールまでの経路を保存するスタック
void Update(Grid<Cell>& grid, std::stack<Point> &stack, std::stack<Point> &route)
{
	if (stack.empty())
	{
		Print << U"NO ROUTE!!";
		return;
	}

	// 1ステップ進める
	Point p = stack.top();
	if (grid[p.y][p.x] == Cell::Goal)
	{
		// ゴールが見つかっているので、スタートからゴールまでの経路を表示する
		while (!route.empty())
		{
			Point r = route.top();
			grid[r.y][r.x] = Cell::Route;
			route.pop();
		}
		return;
	}

	stack.pop(); // ゴールが見つかったら無限ループするようにここで取り出す

	// ブロックなら進まない
	if (grid[p] == Cell::Block) return;

	// 現在地を探索済みにする
	grid[p] = Cell::Finded;
	route.push(p);

	Array<Point> d = { {1,0},{0,1},{-1,0},{0,-1} };
	d.shuffle();
	for (int i : step(4))
	{
		if (grid[p.y + d[i].y][p.x + d[i].x] == Cell::Empty || grid[p.y + d[i].y][p.x + d[i].x] == Cell::Goal)
		{
			Point next = Point(p.x + d[i].x, p.y + d[i].y);
			if(grid[next.y][next.x] == Cell::Empty) grid[next.y][next.x] = Cell::Selected;
			stack.push(next);
		}
	}

}

void Main()
{
	Scene::SetBackground(ColorF{ 0.8, 0.9, 1.0 });

	// 迷路の初期化
	constexpr int32 width = 60;
	constexpr int32 height = 60;

	// [{1,1}, {width, height}] の範囲
	Grid<Cell> grid(width + 2, height + 2, Cell::Empty);
	for (auto y : step(grid.height()))
	{
		for (auto x : step(grid.width()))
		{
			if (x == 0 || y == 0 || x == grid.width() - 1 || y == grid.height() - 1)
			{
				grid[y][x] = Cell::Block;
			}
		}
	}

	// フィールドの初期化
	Image image{ width, height, Palette::Black };
	DynamicTexture texture{ image };
	Stopwatch stopwatch{ StartImmediately::Yes };

	// 変数の宣言
	bool autoStep = false;
	double speed = 0.5;

	bool showGrid = true;
	bool updated = false;

	std::stack<Point> stack;
	std::stack<Point> route;

	while (System::Update())
	{
		// フィールドをランダムな値で埋めるボタン
		if (SimpleGUI::ButtonAt(U"Random", Vec2{ 700, 40 }, 170))
		{
			Point st = RandomFill(grid);
			stack = std::stack<Point>();
			stack.push(st);
			updated = true;
		}

		// 迷路を生成するボタン
		if (SimpleGUI::ButtonAt(U"Maze", Vec2{ 700, 80 }, 170))
		{
			Point st = MakeMaze(grid);
			stack = std::stack<Point>();
			stack.push(st);
			updated = true;
		}

		// フィールドのセルをすべてemptyにするボタン
		if (SimpleGUI::ButtonAt(U"Clear", Vec2{ 700, 120 }, 170))
		{
			stack = std::stack<Point>();
			grid.fill(Cell::Empty);
			updated = true;
		}

		// 一時停止 / 再生ボタン
		if (SimpleGUI::ButtonAt(autoStep ? U"Pause" : U"Run ▶", Vec2{ 700, 160 }, 170))
		{
			autoStep = !autoStep;
		}

		// 更新頻度変更スライダー
		SimpleGUI::SliderAt(U"Speed", speed, 1.0, 0.02, Vec2{ 700, 200 }, 70, 100);

		// 1 ステップ進めるボタン、または更新タイミングの確認
		if (SimpleGUI::ButtonAt(U"Step", Vec2{ 700, 240 }, 170) || (autoStep && stopwatch.sF() >= (speed * speed)))
		{
			Update(grid, stack, route);
			updated = true;
			stopwatch.restart();
		}

		// グリッド表示の有無を指定するチェックボックス
		SimpleGUI::CheckBoxAt(showGrid, U"Grid", Vec2{ 700, 320 }, 170);

		// フィールド上でのセルの編集
		if (Rect{ 0, 0, 599 }.mouseOver())
		{
			const Point target = (Cursor::Pos() / 10 + Point{ 1, 1 });

			if (MouseL.pressed())
			{
				grid[target] = Cell::Block;
				updated = true;
			}
			else if (MouseR.pressed())
			{
				grid[target] = Cell::Empty;
				updated = true;
			}
		}

		// 画像の更新
		if (updated)
		{
			CopyToImage(grid, image);
			texture.fill(image);
			updated = false;
		}

		// 画像をフィルタなしで拡大して表示
		{
			ScopedRenderStates2D sampler{ SamplerState::ClampNearest };
			texture.scaled(10).draw();
		}

		// グリッドの表示
		if (showGrid)
		{
			for (auto i : step(61))
			{
			
				Rect{ 0, i * 10, 600, 1 }.draw(ColorF{ 0.4 });
				Rect{ i * 10, 0, 1, 600 }.draw(ColorF{ 0.4 });
			}
		}

		if (Rect{ 0, 0, 599 }.mouseOver())
		{
			Cursor::RequestStyle(CursorStyle::Hidden);
			Rect{ Cursor::Pos() / 10 * 10, 10 }.draw(Palette::Orange);
		}
	}
}
