# code_OpenGL

## todo
- 構造の改善
    - 頂点とテクスチャの情報を渡す。
    - 渡された情報で描画をできるようにする。

- モード
    - 読み込みファイルの形式から変換方式を変える
    - yuvモードの実装

- 描画更新
- カメラ映像取得
    - ここも魚眼モードを追加したい

- 魚眼→平面化

- 座標変換





## 関数の整理
現在
main

必要なデータの読み込み
    LoadFile

EGL初期処理
    initializeEGL

メインループ
    mainloop


終了処理
destroyEGL(display, context, surface);
XDestroyWindow(xdisplay, xwindow);
XCloseDisplay(xdisplay);



修正

main
初期処理
    LoadFile
    initializeEGL
    init

メインループ = 描画（draw）

glDrawElements
eglSwapBuffers



終了処理