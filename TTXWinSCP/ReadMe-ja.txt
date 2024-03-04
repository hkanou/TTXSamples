TTXWinSCP -- WinSCP起動ランチャー

機能:
  Tera Termに、WinSCPの起動メニューを追加します。
  SCP接続後のディレクトリをサブメニューから選択します。

解説:
  Tera Termの接続情報を使用して、WinSCPを起動します。

    winscp.exe [schema://[username[:password@]hostname:port[/path/]]

  - winscp.exe
    INIファイルのパラメタWinSCPPathの値を使用します。
    未設定の場合は"C:\Program Files (x86)\WinSCP\WinSCP.exe"を使用します。
    パスがホワイトスペースを含む場合は"で囲ってください。
  - schema
    INIファイルのパラメタWinSCPschemaの値を使用します。
    scp、sftp、ftp、ftpsのいずれかを指定してください。
    未設定の場合はscpを使用します。
  - username
    コマンドラインオプション(/user=等)でユーザー名が与えていても、その値は使用されません。
    これは、Process Explorerなどのツールを用いることでWinSCPへの引数が参照可能であり、
    ユーザー名漏洩のきっかけになるためです。
    INIファイルのパラメタWinSCPUserEmbed(デフォルト値:off)にAcceptTheRiskを設定することで
    WinSCPにユーザー名を渡すことは可能ですが、本設定は非推奨です。
    AcceptTheRiskが設定されている場合は、INIファイルのパラメタWinSCPUserの値を使用します。
    コマンドラインオプション(/user=等)でユーザー名が与えられている場合は、その値を優先します。
  - password
    usernameと同様、パスワード漏洩のきっかけになるため、コマンドラインオプション(/passwd=等)で
    パスワードが与えていてもその値は使用されません。
    INIファイルのパラメタWinSCPPasswordEmbed(デフォルト値:off)にAcceptTheRiskを設定することで
    WinSCPにパスワードを渡すことは可能ですが、本設定は非推奨です。
  - host
    接続先のホストを使用します。
  - port
    接続先のポート番号を使用します
    INIファイルのパラメタWinSCPPortに値が設定されている場合は、その値を使用します。
  - path
    INIファイルのパラメタDir1～Dir20の値がサブメニューに表示されます。
    サブメニューの項目数は最大で20です。最大数を増やす場合はTTXWinSCP.cの
    MAX_DIR_ITEMSを増やしてビルドしてください。

  INIファイルのパラメタWinSCPMenuStringの値がメニューのタイトルに表示されます。
  未設定の場合は"WinSCP(&J)"が使われます。

  INIファイルに、[WinSCP]セクションとDirXが未設定の場合は、
  WinSCPの起動メニューは表示されません。

設定例:
  [WinSCP]
  WinSCPMenuString="WinSCP(&J)"
  WinSCPPath="C:\Program Files (x86)\WinSCP\WinSCP.exe"
  WinSCPSchema=scp
  WinSCPPort=
  WinSCPUserEmbed=off
  WinSCPUser=
  WinSCPPasswordEmbed=off
  Dir1=/
  Dir2=/home
  Dir3=/var
  Dir4=/var/log
  Dir5=/var/tmp
  Dir6=/opt
  Dir7=/tmp

改版履歴:
  Ver 0.1 2024/02/25 ベータ版
  Ver 0.2 2024/03/03 初版
  Ver 0.3 2024/03/05 port:接続先のポート番号を優先するよう修正
