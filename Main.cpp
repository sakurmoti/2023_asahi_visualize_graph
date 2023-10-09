# include <Siv3D.hpp> // OpenSiv3D v0.6.10

class Node
{
public:
	int32 m_id;
	int8 m_r = 30;
	Vec2 m_pos;

	Node(int32 id, Vec2 pos) : m_id(id), m_pos(pos) {}

	Circle getCircle() const
	{
		return Circle{ m_pos, m_r };
	}

	void drawNode() const
	{
		getCircle().draw();
	}

	void drawNodeActive() const
	{
		getCircle().draw(ColorF{ 1.0, 0.9, 0.8 });
	}

	void drawId(const Font& font) const
	{
		font(m_id).drawAt(m_pos, ColorF(0.25));
	}

};

void Main()
{
	Scene::SetBackground(ColorF{ 0.8, 0.9, 1.0 });
	const Font font{ 20, Typeface::Bold };

	Array<Node> nodes;
	Optional<int32> activeNodeID;

	while (System::Update())
	{
		if (MouseR.down())
		{
			Print << U"activated is none";
			activeNodeID = none;
		}

		if (MouseL.down())
		{
			Print << U"added Circle";
			nodes.push_back(Node{ (int32)nodes.size(), Cursor::PosF()});
		}




		for (const Node &node : nodes)
		{
			if (node.getCircle().mouseOver())
			{
				Cursor::RequestStyle(CursorStyle::Hand);
			}

			if (node.getCircle().rightClicked())
			{
				activeNodeID = node.m_id;
			}
		}

		for (const Node& node : nodes)
		{
			if (node.m_id == activeNodeID)
			{
				node.drawNodeActive();
			}
			else
			{
				node.drawNode();
			}
			node.drawId(font);
		}
	}
}
