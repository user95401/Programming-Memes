# Programming Memes

Get random endless programming memes images directly in the game using [Programming Memes Images](https://rapidapi.com/kaushalsharma880-GAglnDIvTy/api/programming-memes-images) API.

\- popup meme on game loaded

\- button in geode mods layer to popup meme

\- ability for devs to open random meme using that mod

To open random meme in your mod using that mod:
```cpp
auto openup = typeinfo_cast<CCMenuItem*>(GameManager::get()->getChildByID("popup-random-meme"));
if (openup) openup->activate();
```