/*
  ==============================================================================

    RandomPresetNameGenerator.h
    Created: 22 Dec 2018 1:35:07am
    Author:  Pierre

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class RandomPresetNameGenerator
{
public:
	RandomPresetNameGenerator()
	{
		buildWordLists();
	}

	String get()
	{
		const auto randomAdj = int(random.nextFloat() * adjectives.size());
		String adj = adjectives[randomAdj];
		adj = adj.getCharPointer().toUpperCase() + adj.substring(1);

		const auto randomNoun = int(random.nextFloat() * nouns.size());
		String n = nouns[randomNoun];

		return adj + " " + n;
	}

private:
	void buildWordLists()
	{
		// Random things
		nouns.add("loud");
		nouns.add("boom");
		nouns.add("bam");
		nouns.add("wow");
		nouns.add("war");
		nouns.add("shop");
		nouns.add("sneakers");
		nouns.add("beetle");
		nouns.add("mammal");
		nouns.add("bear");
		nouns.add("lion");
		nouns.add("elephant");
		nouns.add("squid");
		nouns.add("whale");
		nouns.add("seal");
		nouns.add("reptile");
		nouns.add("baboom");
		nouns.add("brawl");
		nouns.add("altercation");
		nouns.add("sex");
		nouns.add("dispute");
		nouns.add("fight");
		nouns.add("riot");
		nouns.add("hassle");
		nouns.add("fuss");
		nouns.add("revolt");
		nouns.add("skank");
		nouns.add("WOW");
		nouns.add("BOW");
		nouns.add("WAW");
		nouns.add("BOOM");
		nouns.add("dose");
		nouns.add("ass");
		nouns.add("idiot");
		nouns.add("bully");
		nouns.add("narcotic");
		nouns.add("pill");
		nouns.add("prescription");
		nouns.add("pharmaceutic");
		nouns.add("candy");
		nouns.add("sugar");
		nouns.add("inspection");
		nouns.add("test");
		nouns.add("clock");
		nouns.add("crap");
		nouns.add("thingy");
		nouns.add("thing");
		nouns.add("opossum");
		nouns.add("youth");
		nouns.add("dream");
		nouns.add("thought");
		nouns.add("trap");
		nouns.add("vision");
		nouns.add("pyramid");
		nouns.add("theory");
		nouns.add("toy");
		nouns.add("animals");
		nouns.add("gang");
		nouns.add("hood");
		nouns.add("group");
		nouns.add("team");

		// Synthesis and computer things
		nouns.add("synth");
		nouns.add("circuit");
		nouns.add("OSC");
		nouns.add("generator");
		nouns.add("electricity");
		nouns.add("digits");
		nouns.add("samples");
		nouns.add("buffer");
		nouns.add("808");
		nouns.add("drive");
		nouns.add("update");
		nouns.add("reboot");

		// Science things
		nouns.add("cell");
		nouns.add("control");
		nouns.add("energy");
		nouns.add("evolution");
		nouns.add("experiment");
		nouns.add("data");
		nouns.add("mass");
		nouns.add("lab");
		nouns.add("tube");
		nouns.add("volume");
		nouns.add("science");
		nouns.add("experiment");
		nouns.add("collision");
		nouns.add("detection");

		// BIG
		adjectives.add("big");
		adjectives.add("super");
		adjectives.add("hyper");
		adjectives.add("mega");
		adjectives.add("adult");
		adjectives.add("astronomical");
		adjectives.add("colossal");
		adjectives.add("deep");
		adjectives.add("chunky");
		adjectives.add("copious");
		adjectives.add("brawny");
		adjectives.add("epic");
		adjectives.add("XXL");
		adjectives.add("XL");
		adjectives.add("fat");
		adjectives.add("giant");
		adjectives.add("gigantic");
		adjectives.add("great");
		adjectives.add("heavy");
		adjectives.add("huge");
		adjectives.add("immense");
		adjectives.add("jumbo");
		adjectives.add("massive");
		adjectives.add("obese");
		adjectives.add("wide");

		// GOOD
		adjectives.add("good");
		adjectives.add("excellent");
		adjectives.add("crack");
		adjectives.add("nice");
		adjectives.add("rad");
		adjectives.add("neat");
		adjectives.add("valuable");
		adjectives.add("boss");
		adjectives.add("worthy");
		adjectives.add("first-rate");
		adjectives.add("precious");
		adjectives.add("admirable");
		adjectives.add("positive");
		adjectives.add("pleasing");
		adjectives.add("superior");
		adjectives.add("tip-top");
		adjectives.add("splendid");
		adjectives.add("superb");
		adjectives.add("marvelous");
		adjectives.add("optimal");
		adjectives.add("bully");
		adjectives.add("ace");
		adjectives.add("spanking");
		adjectives.add("famous");
		adjectives.add("choice");
		adjectives.add("top-notch");
		adjectives.add("great");
		adjectives.add("wonderful");
		adjectives.add("capital");
		adjectives.add("classy");
		adjectives.add("dope");
		adjectives.add("badass");
		adjectives.add("tasty");

		// BAD
		adjectives.add("bad");
		adjectives.add("crazy");
		adjectives.add("dumb");
		adjectives.add("dummy");
		adjectives.add("stupid");
		adjectives.add("silly");
		adjectives.add("sick");
		adjectives.add("ill");
		adjectives.add("terrible");
		adjectives.add("junky");
		adjectives.add("garbage");
		adjectives.add("gross");
		adjectives.add("bummer");
		adjectives.add("abominable");
		adjectives.add("faulty");
		adjectives.add("cheap");
		adjectives.add("awful");
		adjectives.add("atrocious");
		adjectives.add("poor");
		adjectives.add("crappy");
		adjectives.add("tumultuous");
		adjectives.add("dangerous");
		adjectives.add("dark");
		adjectives.add("suspicious");
		adjectives.add("malicious");
		adjectives.add("creepy");
		adjectives.add("mean");
		adjectives.add("darth");
		adjectives.add("deadly");
		adjectives.add("poisonous");
		adjectives.add("weird");
		adjectives.add("strange");


		// Other
		adjectives.add("tight");
		adjectives.add("disco");
		adjectives.add("funky");
		adjectives.add("trap");
		adjectives.add("your");
		adjectives.add("my");
		adjectives.add("his");
		adjectives.add("her");
		adjectives.add("early");
		adjectives.add("late");
		adjectives.add("nightly");
		adjectives.add("confidential");
		adjectives.add("lunatic");
		adjectives.add("surely");
		adjectives.add("like a");
		adjectives.add("more");
		adjectives.add("what the");
		adjectives.add("fck the");
		adjectives.add("oh my");
		adjectives.add("again");
		adjectives.add("more about");
		adjectives.add("what is");
		adjectives.add("about");
		adjectives.add("stressful");
		adjectives.add("ominous");
		adjectives.add("yesterday's");
		adjectives.add("just");
		adjectives.add("this is");
		adjectives.add("cupid");
		adjectives.add("addicted to");
		adjectives.add("lord");
		adjectives.add("king");
		adjectives.add("overdriven");
		adjectives.add("thoughtful");
		adjectives.add("careless");
		adjectives.add("stainless");
		adjectives.add("audacious");
		adjectives.add("bizarre");
	}

	StringArray adjectives;
	StringArray nouns;

	Random random;
};