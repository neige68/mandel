# GoldenMandel - マンデルブロ集合描画

Windows 用のマンデルブロ集合描画アプリです

マンデルブロ集合については
[Wikipedia](https://ja.wikipedia.org/wiki/%E3%83%9E%E3%83%B3%E3%83%87%E3%83%AB%E3%83%96%E3%83%AD%E9%9B%86%E5%90%88)
をご覧ください

MFC より古くからある C++ Windows アプリフレームワーク OWL (Object Windows Library) のアップデートである
[OWLNext](https://sourceforge.net/projects/owlnext/) 
を使用しているため、
ソースからのビルドには
[Object Windows Library のライセンスが必要](https://sourceforge.net/p/owlnext/wiki/Installing_OWLNext/#owl-license)
です。
バイナリには必要ありません。


## スクリーンショット

<img src="https://user-images.githubusercontent.com/89586482/183108358-a574f250-998c-47a0-b8fc-07c73761335b.png" width="384" heighe="227" title="初期ウィンドウ"><img src="https://user-images.githubusercontent.com/89586482/183108388-b99b7be2-2989-468c-a146-e3a1edc08086.png" width="384" heighe="227" title="描画ウィンドウ"><img src="https://user-images.githubusercontent.com/89586482/183108376-d946093d-890b-4240-ad13-64ad5373e346.png" width="334" height="227" title="色設定ダイアログ">

## 特徴

* 操作レスポンス重視で作りました

* 色付けは変更できます。ランダム設定(Ctrl+R)もできます。

* マウスホイールで拡大縮小できます

* マウスドラッグでスクロールできます

* マルチスレッドで高速描画します(シングルコアでも頑張って描きます)

* 生成した画像は JPEG ファイルに保存できます

    作成した JPEG ファイルには、
    Windows エクスプローラで表示・変更可能な「コメント」プロパティに、
    画像生成時のパラメータが記録されています

    生成した画像に対する著作権は主張しません

## 動作環境

Windows Vista 以降

## バイナリリリース使用方法(予定)

好きなディレクトリに解凍して GoldenMandel.exe を実行してください

## 操作

* マウスホイール上回転, PageUp, マウス左ボタンダブルクリック : ズームイン(拡大)

* マウスホイール下回転, PageDown, Shift+マウス左ボタンダブルクリック, マウス右ボタンダブルクリック : ズームアウト(縮小)

    ズームは、
    マウス操作ではマウスポインタ位置を中心に、
    キーボード操作ではウィンドウ中央を中心に、
    2倍(拡大) あるいは 1/2倍(縮小) を繰り返します

* マウス左ボタンでドラッグ, スクロールバー, ←→↑↓ : スクロール(移動)

* Ctrl+R : ランダム色設定

* Ctrl+S : 名前を付けて保存

## ソースからのビルド方法

### 必要なもの

* [Git for Windows](https://gitforwindows.org/)

* [CMake](https://cmake.org/) 3.16 以上

* [Boost](https://www.boost.org/) 1.75 以上

* [OWLNext](https://sourceforge.net/projects/owlnext/) 7.0.2 以上

* [Visual Studio](https://visualstudio.microsoft.com/) 2019 の Visual C++ (VC 14.2)

### ソースファイルを取得

適当なディレクトリで

    git clone git@github.com:neige68/mandel.git

### CMake

パスを通しておいてください

### Boost

src/CMakeLists.txt にて find_package で検索されます

### OWLNext

OWLNext のパスを環境変数 OWLROOT に設定しておくか、
src/CMakeLists.txt にて cmake 変数 OWL に set してください

    set OWLROOT=u:\lib\owl\7.0.2

### Visual C++

VC14.2 の vcvarsall.bat のあるディレクトリを環境変数 VC142 に設定しておくか、
src/build.bat にて環境変数 VC に set してください

    set VC142=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build

### ビルド

src ディレクトリで build.bat を実行してください

    build\debug     - 32ビットデバッグ用
    build\release   - 32ビットリリース用
    build64\debug   - 64ビットデバッグ用
    build64\release - 64ビットリリース用
    
と分かれてビルドされます

64ビットの利点は今のところないと思います

## ライセンス

とりあえず GPLv3 で公開しております

## 今後の予定

* 拡大縮小の連続化

* パラメータを変更可能にする

* GPU 使用(OpenCL, Boost Compute 予定, float 対応)

* 回転

* 反復回数を自動で決める

* 漸化式を指定可能にする(Boost Spirit X3 予定, または OpenCL に投げるだけ)

## 連絡先

* [github issues](https://github.com/neige68/mandel/issues)

* twitter julia @julia230348171
