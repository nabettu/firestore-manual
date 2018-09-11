= はじめに

「Vue.jsとFirebaseで作るミニWebサービス ~初めてのサーバーレスシングルーページアプリケーション！~」（以下、Webサービス作る本）ではFirebaseを利用してユーザー登録やサイトホスティング、またユーザーデータの保存を利用していました。

//quote{
Vue.jsとFirebaseで作るミニWebサービス

@<href>{https://nextpublishing.jp/book/9884.html}
//}

本の内容でデータ保存には「Realtime Database（以下RTDB）」を利用していましたが、後継として「Firestore」という別なデータベースがあります。
執筆時にはまだβ版で不安定だったため不採用にしたFirestoreですが、最近は動作も安定しており本番環境で採用している企業も増えてきました。個人的には今年の末〜来年の頭までにはβ版が解除されるのではないかと思っています。（本マニュアル執筆:2018年9月）

FirestoreはRTDBに比べてさまざまな機能が追加されており、Webサービス作る本で利用するような通常のデータベースであれば、今後は基本的にFirestoreを使ったほうがよいと考えています。

本マニュアルではRTDBとの違いや、Webサービス作る本においてFirestoreを使った場合の実装への移行について書いていきますと思います。

= RTDBとFirestoreの違いについて

現在Firestoreでは、東京リージョンが選択出来ない（予定はされています）ため、動作で少しタイムラグが発生します。そのため、RTDBはリアルタイムにデータのやりとりを行う場面ではFirestoreよりも優れているため、場面によって使い分けていただければと思います。

== データの保存形式

RTDBでは全体のデータを1つの大きなJSONツリーとして保存します。シンプルなデータは非常に簡単に保存できますが、複雑で階層的なデータを処理する場合は色々と工夫が必要でした。

Firestoreでも保存するデータは概ね、JSONに似ています。
Firestoreはコレクションというデータのまとまりにまとめ、内部データをドキュメントと呼び、コレクションごとに個別でクエリを発行して検索などができるなどがRTDBと大きく違う点です。

データを階層化しても、その階層化のデータをまたサブコレクションとすることで大量のデータも処理しやすくなりました。それによりRTDBのデータを利用する際に必要だった平滑化などのテクニックがなくともさまざまな処理をやりやすくなりました。

さらにデータに型を定義できたり、リファレンス型という他のコレクションやドキュメントのデータリファレンスを格納することができるようになり、リレーショナルなデータ格納が可能になりました。

== ルール設定の違い

RTDBでは基本的にreadとwriteの2パターンについてのルールの設定が可能で、validationでそれを補う形のでしたが、
Firestoreではread・writeの他にそれぞれをさらに詳細にget・list、create・update・deleteごとに設定をすることができるようになったり、自分自身以外のデータを取得して比較するなども比較的楽になりました。

また、ルール内で@<b>{カスタム関数}を定義できるようになり、複雑化するルールのチェックをより簡潔に記述できるようになりました。

//image[cunstomfunction][カスタム関数の例][scale=0.8]{
//}

== 料金の違い

RTDBは帯域幅とストレージにのみ課金でしたが、Firestoreはデータベースで実行されているオペレーション（読み取り、書き込み、削除）に課金され、帯域幅とストレージの課金レートは低くなります。
また、Google App Engineと連携させて、１日の限度額を設定できるようにもなりました。

== その他の違い

RTDBでは膨大なデータが保存されている場合にはスケールする際にシャーディングが必要となっていましたが、Firestoreではスケーリングの際にそういった壁はありません。

= RTDBからの移行作業

Vue.jsとFirebaseで作るミニWebサービスの初版P43 「5.4 Firebase Realtime DBのruleを設定する」をFirestoreで設定する場面から順に進めていきます。

== 初期設定

まずはFirebaseの管理画面から今回のプロジェクトで利用しているプロジェクトを開きます。

左のメニューからDatabaseを選択するとRTDBのデータ画面が表示されると思います。

※もし新規プロジェクトで作成する場合はFirestoreが表示されていると思いますので、データベースの作成を選択してください。

データベースの管理画面の上部にある「Realtime Database」と表示してあるセレクトボックスをクリックすると「Cloud Firestore」がありますので、それを選択します。

//image[selectbox][セレクトボックスをクリックした状態][scale=0.8]{
//}

その後セキュリティルールの初期設定をどうするか表示されますが、こちらは後ほど変更するためロックモードのまま次へ進みます。

//image[securityrule][セキュリティルールの選択][scale=0.8]{
//}

これでデータベースとして利用するFirestoreのセットアップは完了です。次にRTDBと同じようにルールの設定を行います。

== ルール設定

FirestoreではRTDBでのルール設定と比べて複雑なルールも比較的簡単に記述しやすくなりました。また、RTDBのみであったルール設定の確認のためのシミュレータも2018年6月から実装されたことでルールの検証がしやすくなりました。

公式ドキュメントは次になります。

//quote{
Cloud Firestore セキュリティルールを使ってみる

@<href>{https://firebase.google.com/docs/firestore/security/get-started}
//}

管理画面のルールタブを選択し、ルール設定画面を表示します。

RTDBで設定した場合と同じようにmemosというコレクションを作成し、配下に認証時のユーザーID(（user_id）と同じ名称でドキュメントを作成します。そして、そのドキュメント配下は自分自身しか読み書き出来ないように設定します。

※RTDBでは"auth"に認証情報が格納されていましたが、Firestoreのルール設定では"request.auth"になっているので注意が必要です。

//list[source-code][セキュリティルール][text]{
service cloud.firestore {
  match /databases/{database}/documents {
    match /memos/{user_id} {
      allow read, write: if request.auth.uid == user_id;
    }
  }
}
//}

== ルールシュミレータの実行

Firestoreでもルールシュミレータが実装されたので、試しに利用してみましょう。

ルール管理画面の左の星の辺りにマウスオーバーすると、ルール設定の履歴が閲覧できます。下にあるシミュレータボタンをタップしてみましょう。

//image[simurateBtn][シミュレータの実行][scale=0.8]{
//}

シミュレーションタイプではgetやcreate等それぞれテストができます。今回はmemos配下のuser_idのドキュメントが読み込み出来ないかのシミュレーションを試します。

ここで自身の認証ユーザーIDが知りたいので、一度Authenticationタブを開きます。そこでは認証済のユーザーの一覧が見られますので、自分のメールアドレスの「ユーザーUID」をコピーします。

//image[auth][認証情報一覧][scale=0.8]{
//}

もう一度ルールシミュレータに戻り「場所」の部分に「memos/コピーしてきたユーザーID」を入力します。認証済みボタンをOFFのまま触らず「実行」ボタンを押してみてください。

「 Error: simulator.rules line [4], column [29]. Null value error.」という表示がされたと思います。これは認証がされていないユーザーがDBの読み込みを行おうとして、認証情報が入っている"request.auth"が空のためエラーになって読み込みに失敗（データが正しく保護されている）しています。

//image[auth_error][認証していないユーザーのアクセス][scale=0.8]{
//}

次は「認証済み」のボタンをONにして、Firebase UIDの部分にコピーしてきたIDと別なIDを入力して、実行してみましょう。
今度は「シミュレーションによるデータアクセスは許可されませんでした」と、出ました。認証情報が一致しないことで読み込みに失敗しています。

//image[auth_error2][自分以外のユーザーのアクセス][scale=0.8]{
//}

最後に認証済みのユーザーをDBのIDと一致させて実行してみて下さい。
「シミュレーションによるデータアクセスは許可されました」と表示されたらチェックOKです。データが正しく保護されていることを確認できました。

//image[auth_ok][アクセス成功][scale=0.8]{
//}

ではルールの設定ができたので、データの読み取りと保存の実装に移りましょう。

== データ保存の実装

Vue.jsとFirebaseで作るミニWebサービスの初版P45「5.5 メモの保存と読み込み機能の作成」をFirestoreに移行します。

まずFirestoreを利用するためには、Firestore用のSDKを別途読み込む必要があります。

//quote{
Cloud Firestore を使ってみる

@<href>{https://firebase.google.com/docs/firestore/quickstart}
//}

こちらのFirestoreを使ってみるの「開発環境の設定」で「ウェブ」のタブにあるScriptをコピーして、index.htmlのscriptを置き換えましょう。

//list[source-code1][/dist/index.htmlのfirebase.jsの読み込み部分を置き換え][html]{
<script src="https://www.gstatic.com/firebasejs/4.12.1/firebase.js"></script>
<script src="https://www.gstatic.com/firebasejs/4.12.1/firebase-firestore.js">
</script>
//}

次にEditor.vueのsaveMemos関数の中身をFirestore対応したものに置き換えます。

//list[source-code2][/src/components/Editor.vue][javascript]{
saveMemos: function() {
  firebase
    .firestore()
    .collection("memos")
    .doc(this.user.uid)
    .set({ memos: this.memos });
},
//}

RTDBの場合はrefでデータベースのリファレンスを直接指定していましたが、Firestoreではコレクション.ドキュメントという形式での指定が必要です。

また、このとき.setの中身はドキュメントへの保存となり、オブジェクトである必要があるためthis.memosをそのまま保存しようとするとエラーになってしまいます。そのため、memosというキーのオブジェクトに保存するように設定しました。

管理画面でデータがFirestoreへ保存されていることが確認できたら次は読み取りへ移ります。

//image[datasave][管理画面でのFirestoreのデータが保存されたことの確認][scale=0.8]{
//}

== データ読み込みの実装

次は読み込み部分です、保存の際と同じようにリファレンスを作成してかたgetする形になります。
また、保存の際にmemosというキーでオブジェクトへ変換したので、読み取り時にもオブジェクトのデータを読みに行く必要があります。

//list[source-code3][/src/components/Editor.vue][javascript]{
created: function() {
  firebase
    .firestore()
    .collection("memos")
    .doc(this.user.uid)
    .get()
    .then(doc => {
      if (doc.exists && doc.data().memos) {
        this.memos = doc.data().memos;
      }
    });
},
//}

RTDBと同じようにリロードしてもデータが自動的に読み取り、表示されましたでしょうか。以上でFirestoreへの移行作業は終了となります。お疲れ様でした。

== おわりに

いかがでしたでしょうか。RTDBと基本的な使い方は大きく変わらないため、移行作業自体は結構すぐに終わったと思います。

今後もFirebaseではFirestore以外にも便利な機能が増えていくと思われます。様々な機能を使いこなし得手不得手を理解して取捨選択しながら便利に利用して、面白いWebサービスを開発していっていただければと思います。

本マニュアルがそのための一助になれば幸いです。最後までお読みいただきありがとうございました。

なにか誤字・誤植等見つけられた方はTwitterで@nabettu宛にご連絡いただければと思います。