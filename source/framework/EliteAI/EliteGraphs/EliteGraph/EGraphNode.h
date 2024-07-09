#pragma once
#include "EGraphEnums.h"
#include "../EliteGraphUtilities/EGraphVisuals.h"
namespace Elite
{
	class GraphNode
	{

	public:
		GraphNode(const Elite::Vector2& pos = Elite::ZeroVector2);
		virtual ~GraphNode() = default;

		int GetId() const { return m_Id; }

		Elite::Vector2 GetPosition() const { return m_Position; }
		void SetPosition(const Elite::Vector2& newPos) { m_Position = newPos; }

		Elite::Color GetColor() const { return m_Color; }
		void SetColor(const Elite::Color& color) { m_Color = color; }



	protected:
		int m_Id;
		Elite::Vector2 m_Position;
		Elite::Color m_Color;

		friend class Graph;
		void SetId(int id) { m_Id = id; }

	};



}