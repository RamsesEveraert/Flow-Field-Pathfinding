#pragma once

#include <vector>
#include "framework/EliteGeometry/EGeometry2DTypes.h"
#include "framework/EliteAI/EliteGraphs/EliteNavGraph/ENavGraphNode.h"

namespace Elite
{
	//Portal struct (only contains line info atm, you can expand this if needed)
	struct Portal
	{
		Portal() {}
		explicit Portal(const Elite::Line& line) :
			Line(line)
		{
		}
		Elite::Line Line = {};
	};


	class SSFA final
	{
	public:
		//=== SSFA Functions ===
		//--- References ---
		//http://digestingduck.blogspot.be/2010/03/simple-stupid-funnel-algorithm.html
		//https://gamedev.stackexchange.com/questions/68302/how-does-the-simple-stupid-funnel-algorithm-work
		static std::vector<Portal> FindPortals(const std::vector<GraphNode*>& nodePath, Polygon* navMeshPolygon)
		{
			//Container
			std::vector<Portal> vPortals = {};

			vPortals.emplace_back(Portal(Line(nodePath[0]->GetPosition(), nodePath[0]->GetPosition())));

			//For each node received, get it's corresponding line
			for (size_t i = 1; i < nodePath.size() - 1; ++i)
			{
				//Local variables
				auto pNode = static_cast<NavGraphNode*>(nodePath[i]); //Store node, except last node, because this is our target node!
				auto pLine = navMeshPolygon->GetLines()[pNode->GetLineIndex()];

				//Redetermine it's "orientation" based on the required path (left-right vs right-left) - p1 should be right point
				auto centerLine = (pLine->p1 + pLine->p2) * 0.5f;
				auto previousPosition = i == 0 ? nodePath[0]->GetPosition() : nodePath[i - 1]->GetPosition();
				auto cp = Cross((centerLine - previousPosition), (pLine->p1 - previousPosition));
				Line portalLine = {};
				if (cp > 0)//Left
					portalLine = Line(pLine->p2, pLine->p1);
				else //Right
					portalLine = Line(pLine->p1, pLine->p2);

				//Store portal
				vPortals.emplace_back(Portal(portalLine));
			}
			//Add degenerate portal to force end evaluation
			vPortals.emplace_back(Portal(Line(nodePath[nodePath.size() - 1]->GetPosition(), nodePath[nodePath.size() - 1]->GetPosition())));

			return vPortals;
		}
        static std::vector<Elite::Vector2> OptimizePortals(const std::vector<Elite::Portal>& portals)
        {
            std::vector<Elite::Vector2> vPath = {};

            const unsigned int amtPortals{ static_cast<unsigned int>(portals.size()) };

            unsigned int apexIndex = 0, leftLegIndex = 1, rightLegIndex = 1;
            Elite::Vector2 apexPos = portals[apexIndex].Line.p1;
            Elite::Vector2 rightLeg = portals[rightLegIndex].Line.p1 - apexPos;
            Elite::Vector2 leftLeg = portals[leftLegIndex].Line.p2 - apexPos;

            // Add the apexPoint to the path (first path point)
            vPath.push_back(apexPos); 

            // Loop over all the portals (Starting from the second portal)
            for (unsigned int portalIdx = 1; portalIdx < amtPortals; ++portalIdx)
            {
                //Get the current portal and store it in a local variable
                const auto& currentPortal = portals[portalIdx];
               
                // Right Check
                Elite::Vector2 newRightLeg = currentPortal.Line.p1 - apexPos;

                // Check if going inwards (CCW)
                if (Cross(newRightLeg, rightLeg) <= 0)
                {
                    // If going inwards
                    // Check if we cross over the leftLeg
                    if (Cross(newRightLeg, leftLeg) < 0)
                    {
                        // If we do cross over the leftLeg

                        apexPos += leftLeg; // Move the apexPos by adding the leftLeg to the apexPos
                        apexIndex = leftLegIndex; // Set apexIndex to be the leftLegIndex
                        portalIdx = leftLegIndex + 1; // Set portalIdx to be leftLegIndex +1 (next portal to check)
                        
                        // Set leftLegIndex and rightLegIndex to be equal to that new portalIdx
                        leftLegIndex = portalIdx;
                        rightLegIndex = portalIdx;

                        // Push the current (new) apex point to the path
                        vPath.push_back(apexPos);

                        // Using the new index, calculate the new legs if it smaller than the size of the portals:
                        if (portalIdx < amtPortals)
                        {
                            rightLeg = portals[rightLegIndex].Line.p1 - apexPos;
                            leftLeg = portals[leftLegIndex].Line.p2 - apexPos;
                            continue;
                        }
                    }
                    else
                    {
                        // If not crossing over the leftLeg 
                        rightLeg = newRightLeg;
                        rightLegIndex = portalIdx;
                    }
                }

                // Left Check
                Elite::Vector2 newLeftLeg = currentPortal.Line.p2 - apexPos;

                // Check if going inwards (CCW)
                if (Cross(newLeftLeg, leftLeg) >= 0)
                {
                    // If going inwards
                    // Check if we cross over the rightleg
                    if (Cross(newLeftLeg, rightLeg) > 0)
                    {
                        // If we do cross over the rightleg

                        apexPos += rightLeg; // Move the apexPos by adding the rightleg to the apexPos
                        apexIndex = rightLegIndex; // Set apexIndex to be the rightlegIndex
                        portalIdx = rightLegIndex + 1; // Set portalIdx to be rightlegIndex +1 (next portal to check)

                        // Set leftLegIndex and rightLegIndex to be equal to that new portalIdx
                        leftLegIndex = portalIdx;
                        rightLegIndex = portalIdx;

                        // Push the current (new) apex point to the path
                        vPath.push_back(apexPos);

                        // Using the new index, calculate the new legs if it smaller than the size of the portals:
                        if (portalIdx < amtPortals)
                        {
                            rightLeg = portals[rightLegIndex].Line.p1 - apexPos;
                            leftLeg = portals[leftLegIndex].Line.p2 - apexPos;
                            continue;
                        }
                    }
                    else
                    {
                        // If not crossing over the rightleg 
                        leftLeg = newLeftLeg;
                        leftLegIndex = portalIdx;
                    }
                }
            }

            // Push the last point to the path 
            vPath.push_back(portals.back().Line.p1);

            return vPath;
        }


	private:
		SSFA() {};
		~SSFA() {};
	};
}
