# Finishing touches

In this last entry, we'll add the final few touches: a basic character creation screen, and the death screen. Both of these screens are separate game states.

![Character creation screen](img/charcreate.png?raw=true "Character creation screen")

## Character screen

Chances are that you want to have some control over the character creation process. This means we need some interactive screen that we can configure our new character. To keep this simple for this tutorial, the only thing you can choose is the name.

The creation of the new state is similar to what we've done before, and looks like this:

```cpp
// When we start a new game, we end up in this state
class CreateChar : public State
{
public:
  CreateChar(std::function<void(bool, const State*)> onDone) : State(onDone) {}

  const std::string& CharName() const { return charName; }
private:
  void Render() override;
  Status UpdateImpl() override;
private:
  // the name of the character that we've entered
  std::string charName;

  // if this is true, rebuild this screen
  bool isGuiDirty = true;
};
```

The element of interest here is the ```charName``` variable, which will store the character name. We override the update and render functions, to handle input and display.

The input handling can be summarized as follows:
* If we press ESC, we cancel the state (and go back to main menu)
* If we press a valid character, we add this character to charName (also rebuild graphics!)
* If we press backspace and the charName is not empty, we remove the last character (also rebuild graphics!)
* If we press enter, we set this state to complete

In terms of rendering, we need to generate our font map, and we do that the first time we enter this state, and every time we've asked to rebuild the graphics (when adding/removing characters). You can see from the image above that not much is going on: there's a single line with the label and the character name, and another line with the press enter to continue message. As with other states, we request a sparse buffer to write to, initialize it if necessary, and set its data after we construct them. We render the buffer using the ```RenderMenu``` method, which utilizes the entire screen.

![Death screen](img/death.png?raw=true "Character creation screen")

## Death screen

The death screen is even simpler in terms of architecture, as it doesn't need to store any data that any other state will need to use:

```cpp
// When the player dies, we end up in this state
class Death : public State
{
private:
  // Display death info
  void Render() override;
  // wait for ENTER key to go back to menu
  Status UpdateImpl() override;
private:
  // set this to true if we need to rebuild the view
  bool isGuiDirty = true;
};
```

In terms of input, we just accept the enter key, and if we press it we end the state, moving back to the main menu.
In terms of rendering, we have a special header line: "You have died", we have special content (where did we die, how many creatures we killed etc) that we present in the main game area, and we leave the character info area as it was at the time of death. So, as previously, we retrieve the appropriate buffers that we want to update (header and death), we build the data and send them to the GPU, and we render all 3 elements: header, body and character info.

## Menu state changes

Menu gets some extra code to deal with these 2 new states. First, instead of starting a new game using the appropriate shortcut, we start the CreateChar state that calls StartGame when it's successful (== we have a valid name set):

```cpp
std::unique_ptr<State> newState(new CreateChar([&](bool success, const State* state) {
if (success)
{
  const auto& charName = static_cast<const CreateChar*>(state)->CharName();
  StartNewGame(charName);
}
}));
```

The other change is with regards to what happens when we get the message that the player dies. First, we need to create the signal in signals.h 

```cpp
static Nano::Signal<void()> onPlayerDied;
```

This signal is triggered in the ```ModifyHp``` function, if the entity is dead and is the player.

We need to listen to that signal in the menu state with appropriate code, and in the listener function we need to  we set a flag ```changeToDeathState``` to true, and next time we run our update function, if that flag is true (== the player is dead), we create and push this new state:

```cpp
if (changeToDeathState)
{
  changeToDeathState = false;
  std::unique_ptr<State> newState(new state::Death());
  Game::Instance().PushState(newState);
}
```

As a minor note, while this ```changeToDeathState``` is true we should not render anything, as otherwise the game might sneak in an image of the menu screen before it goes into the update method and switches to the death state.

And that's all!

