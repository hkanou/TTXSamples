TTXWinSCP -- WinSCP�N�������`���[

�@�\:
  Tera Term�ɁAWinSCP�̋N�����j���[��ǉ����܂��B
  SCP�ڑ���̃f�B���N�g�����T�u���j���[����I�����܂��B

���:
  Tera Term�̐ڑ������g�p���āAWinSCP���N�����܂��B

    winscp.exe [schema://[username[:password@]hostname:port[/path/]]

  - winscp.exe
    INI�t�@�C���̃p�����^WinSCPPath�̒l���g�p���܂��B
    ���ݒ�̏ꍇ��"C:\Program Files (x86)\WinSCP\WinSCP.exe"���g�p���܂��B
    �p�X���z���C�g�X�y�[�X���܂ޏꍇ��"�ň͂��Ă��������B
  - schema
    INI�t�@�C���̃p�����^WinSCPschema�̒l���g�p���܂��B
    scp�Asftp�Aftp�Aftps�̂����ꂩ���w�肵�Ă��������B
    ���ݒ�̏ꍇ��scp���g�p���܂��B
  - username
    �R�}���h���C���I�v�V����(/user=��)�Ń��[�U�[�����^���Ă��Ă��A���̒l�͎g�p����܂���B
    ����́AProcess Explorer�Ȃǂ̃c�[����p���邱�Ƃ�WinSCP�ւ̈������Q�Ɖ\�ł���A
    ���[�U�[���R�k�̂��������ɂȂ邽�߂ł��B
    INI�t�@�C���̃p�����^WinSCPUserEmbed(�f�t�H���g�l:off)��AcceptTheRisk��ݒ肷�邱�Ƃ�
    WinSCP�Ƀ��[�U�[����n�����Ƃ͉\�ł����A�{�ݒ�͔񐄏��ł��B
    AcceptTheRisk���ݒ肳��Ă���ꍇ�́AINI�t�@�C���̃p�����^WinSCPUser�̒l���g�p���܂��B
    �R�}���h���C���I�v�V����(/user=��)�Ń��[�U�[�����^�����Ă���ꍇ�́A���̒l��D�悵�܂��B
  - password
    username�Ɠ��l�A�p�X���[�h�R�k�̂��������ɂȂ邽�߁A�R�}���h���C���I�v�V����(/passwd=��)��
    �p�X���[�h���^���Ă��Ă����̒l�͎g�p����܂���B
    INI�t�@�C���̃p�����^WinSCPPasswordEmbed(�f�t�H���g�l:off)��AcceptTheRisk��ݒ肷�邱�Ƃ�
    WinSCP�Ƀp�X���[�h��n�����Ƃ͉\�ł����A�{�ݒ�͔񐄏��ł��B
  - host
    �ڑ���̃z�X�g���g�p���܂��B
  - port
    �ڑ���̃|�[�g�ԍ����g�p���܂�
    INI�t�@�C���̃p�����^WinSCPPort�ɒl���ݒ肳��Ă���ꍇ�́A���̒l���g�p���܂��B
  - path
    INI�t�@�C���̃p�����^Dir1�`Dir20�̒l���T�u���j���[�ɕ\������܂��B
    �T�u���j���[�̍��ڐ��͍ő��20�ł��B�ő吔�𑝂₷�ꍇ��TTXWinSCP.c��
    MAX_DIR_ITEMS�𑝂₵�ăr���h���Ă��������B

  INI�t�@�C���̃p�����^WinSCPMenuString�̒l�����j���[�̃^�C�g���ɕ\������܂��B
  ���ݒ�̏ꍇ��"WinSCP(&J)"���g���܂��B

  INI�t�@�C���ɁA[WinSCP]�Z�N�V������DirX�����ݒ�̏ꍇ�́A
  WinSCP�̋N�����j���[�͕\������܂���B

�ݒ��:
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

���ŗ���:
  Ver 0.1 2024/02/25 �x�[�^��
  Ver 0.2 2024/03/03 ����
  Ver 0.3 2024/03/05 port:�ڑ���̃|�[�g�ԍ���D�悷��悤�C��
