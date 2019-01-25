/*
  ==============================================================================

    AudioChainComponent.h
    Created: 25 Jan 2019 12:48:31pm
    Author:  Pierre

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class AudioChainComponent : public Component
{
public:
	enum ColourIds
	{
		blockOutlineColourId = 0x100000,
		blockBackgroundColourId,
		blockTextColourId,
		cableColourId,
		hoverBackgroundColourId,
		hoverTextColourId
	};


	AudioChainComponent()
	{
		setColour(blockOutlineColourId, Colours::black);
		setColour(blockBackgroundColourId, Colours::white);
		setColour(hoverBackgroundColourId, Colours::grey);
		setColour(blockTextColourId, Colours::black);
		setColour(hoverTextColourId, Colours::black);
		setColour(cableColourId, Colours::black);
	}

	void paint(Graphics &g) override
	{
		// Draw cable
		g.setColour(findColour(cableColourId));
		auto w = blocks.size() * (blockWidth + spacing);
		g.fillRect(getLocalBounds().removeFromLeft(w).withSizeKeepingCentre(w, 1));

		auto r = getLocalBounds();

		// Draw blocks
		for (auto block : blocks)
		{
			auto b = r.removeFromLeft(blockWidth + spacing).withSizeKeepingCentre(blockWidth, blockHeight);

			auto bg = block->hovered ? findColour(hoverBackgroundColourId) : findColour(blockBackgroundColourId);
			g.setColour(bg);
			g.fillRoundedRectangle(b.toFloat(), corner);

			g.setColour(findColour(blockOutlineColourId));
			g.drawRoundedRectangle(b.toFloat(), corner, 1.0f);

			auto txt = block->hovered ? findColour(hoverTextColourId) : findColour(blockTextColourId);
			g.setColour(txt);
			g.drawFittedText(block->label, b, Justification::centred, 1);
		}
	}

	void resized() override
	{
	}
	
	void mouseMove(const MouseEvent& e) override
	{
		auto r = getLocalBounds();

		for (auto block : blocks)
		{
			auto b = r.removeFromLeft(blockWidth + spacing).withSizeKeepingCentre(blockWidth, blockHeight);

			if (! block->canMove)
				continue;

			block->hovered = b.contains(e.position.x, e.position.y);
		}

		repaint();
	}

	void mouseExit(const MouseEvent& e) override
	{
		draggedBlock = nullptr;

		for (auto block : blocks)
			block->hovered = false;

		repaint();
	}

	void mouseDown(const MouseEvent& e) override
	{
		auto block = getBlockAt(e.getPosition());

		if (block->canMove)
			draggedBlock = block;
	}

	void mouseUp(const MouseEvent& e) override
	{
		draggedBlock = nullptr;
	}

	void mouseDrag(const MouseEvent& e) override
	{
		if (draggedBlock == nullptr)
			return;

		// Desired position
		auto newPosition = blocks.indexOf(getBlockAt(e.getPosition()));

		if (newPosition < 0)
			return;

		auto currentPosition = blocks.indexOf(draggedBlock);

		// Snap to nearest valid position
		while (!blocks[newPosition]->canMove && newPosition != currentPosition)
		{
			if (newPosition > currentPosition)
				newPosition--;
			else
				newPosition++;
		}

		if (newPosition != currentPosition)
		{
			blocks.move(currentPosition, newPosition);
			repaint();
			changed();
		}
	}

	void addBlock(int idx, String label, bool canMove, int insertPos = -1)
	{
		blocks.insert(insertPos, new Block { idx, label, canMove });
	}

	std::function<void()> changed;

	Array<int> getLayout() 
	{
		Array<int> indexes;

		for (auto b : blocks)
			indexes.add(b->index);

		return indexes;
	}

private:
	struct Block
	{
		const int index;
		const String label;
		const bool canMove;

		bool hovered = false;
	};

	OwnedArray<Block> blocks;

	Block* draggedBlock = nullptr;

	Block* getBlockAt(const Point<int>& pos)
	{
		auto r = getLocalBounds();

		for (auto block : blocks)
		{
			auto b = r.removeFromLeft(blockWidth + spacing).withSizeKeepingCentre(blockWidth, blockHeight);
			
			if (b.contains(pos.x, pos.y))
				return block;
		}

		return nullptr;
	}

	int blockWidth = 30;
	int blockHeight = 20;
	int spacing = 6;
	float corner = 3.0f;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioChainComponent)
};