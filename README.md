# DeepOnePlayer

## はじめに
画像や動画を表示しつつ音声を再生します。

- Backend
  - Media Foundation
  - WIC
  - Direct2D

## 再生方法
メニュー欄`Folder->Open`から次のような構造のフォルダを開くと再生を開始します。
<pre>
10345304
├ 101.jpg
├ 102.jpg
├ ...
├ 10345304.txt
├ adu103453_01_01.mp3
├ adu103453_01_02.mp3
└ ...
</pre>
<pre>
10201005
├ 102010_01.mp4
├ 102010_02.mp4
├ ...
├ 10201005.txt
├ adu102010_02_01.mp3
├ adu102010_02_02.mp3
└ ...
</pre>

## マウス機能 

| 入力 | 機能 |
----|---- 
マウスホイール| 拡大・縮小。
左ボタンクリック| 次の静画・動画を表示。
左ボタンドラッグ| 表示区域移動。ディスプレイ解像度以上に拡大した場合のみ動作。
中ボタン|(静画) 原寸大表示。<br>(動画) 基準尺度表示。
右ボタン + マウスホイール|音声送り・戻し。
右ボタン + 中ボタン|窓枠消去・表示。消去時にはディスプレイ原点位置に移動。
右ボタン + 左ボタンクリック|窓移動。窓枠消去時のみ動作。

## キーボード機能

| 入力 | 機能 |
----|---- 
Esc| 終了。
| Up | 次のフォルダを開く。 |
| Down | 前のフォルダを開く。 |
| C   | 文字色黒・白切り替え。 |
| T   | 文章表示・非表示切り替え。 |

## メニュー機能
| 分類 | 項目 | 機能 |
----|---- |---- 
Folder| Open| 再生フォルダ選択。
Audio| Loop| 音声ループ有効・無効切り替え。
-| Setting| 音声音量・再生速度設定画面表示。
Video| Pause| 動画一時停止。
-| Setting| 動画再生速度設定画面表示。
