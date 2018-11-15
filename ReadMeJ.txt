CreditNumberRecognizer

2018/11/14 Version 1.2
- コマンドラインから直接実行できる機能追加
- カメラキャプチャから使用する機能を追加
- CMakeFile.txtの追加
- 対応OpenCVのバージョンアップ

2015/04/26 First Version

--------------------
本プログラムは画像からクレジットカード番号を読み取ります。
クレジットカードをできるだけ画面いっぱいに、傾きがないように撮影して、画像を保存して下さい。

CreditNumberRecognizer.zipを解凍すると、Windows版の実行ファイルをすぐ利用できます。（要VisualStuido2017ランタイムライブラリ）

尚、本プログラムは以下の環境で動作確認されました。
- Windows10 & CentOS7.4
- Visual Stuido2017
- OpenCV 3.4.3
- Boost 1.68

ビルド方法(Linux)：
事前にOpenCV(https://opencv.org/)と(https://www.boost.org/)がインストールされているものとします。
$ git clone https://github.com/takmin/CreditNumberRecognizer.git
$ cd CreditNumberRecognizer
$ mkdir build
$ cd build
$ cmake ..
$ make
ここで、CreditNumberRecognizerという実行ファイルが生成されます。


使用手順（インタラクションモード）：
1. CreditNumberRecognizerを起動
2. "command:"というプロンプトが出るので、"load"と打ち込みリターン
3. "Classifier File:"というプロンプトが出るので、同梱した"CreditModel.txt"への明日を指定
4. "command:"プロンプトで、"recog"とうちリターン
5. "Image File Name:"プロンプトで、クレジットカード画像へのファイルパスを指定。
6. "Save File Name:"プロンプトで、出力画像名を記述。この名前で結果が保存されます。
7. "exit"でプログラムを終了

その他：
・"recog_folder"と入力し、"Directory Name:"プロンプトでフォルダ名を指定すると、指定したフォルダ内にある画像を全て一括で処理してくれます。出力先は"Save Directory"で指定したフォルダの中に保存されます。
・"recog_capture"と入力すると、カメラキャプチャが画面が立ち上がります。枠内にクレジットカードを入れて何かキーを押すと、画像がキャプチャされて番号が認識されます。
・コマンドプロンプトで"h"とうつと使用可能なコマンド一覧が出ます。


コマンドラインから直接実行：
・起動時に引数を指定することで、インタラクションなしに、コマンドラインから直接実行できます。
---
CreditNumberRecognizer input [option]
option:
  -i [ --input ] arg                    入力画像または画像の入ったフォルダへのパス
  -h [ --help ]                         ヘルプの表示
  -m [ --model ] arg (=CreditModel.txt) モデルファイルを指定
  -o [ --output ] arg                   認識結果を画像として保存。inputがフォルダの時はフォルダへのパス
  -c [ --camera ]                       Webカメラの入力を使用
----

注意：
・今回、エラー処理などは組み込んでいませんので、イレギュラーな使用法で落ちることがあります。
・画像にはクレジットカード全体が写ってあり、余計な背景等の映り込みがないものとします。

Hope to do：
・カードの傾き補正の実装
・文字認識の精度向上（要学習データ）


株式会社ビジョン＆ITラボ　
皆川卓也 (http://visitlab.jp)