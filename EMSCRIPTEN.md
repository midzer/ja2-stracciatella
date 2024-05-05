# Emscripten

## Build

```
emmake make
```

## Link

```
emcc -flto -O3 */*.o */*/*.o -o index.html -sUSE_SDL=2 -sASYNCIFY --preload-file Data/ --preload-file ja2.ini --preload-file Ja2.set --preload-file NoIntro.txt -sINITIAL_MEMORY=256mb -sSTACK_SIZE=262144 --closure 1 -sEXPORTED_RUNTIME_METHODS=['allocate']
```
