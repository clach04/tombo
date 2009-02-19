set PERL=c:\perl\bin\perl
%PERL% GenDefaultResource.pl
%PERL% CheckGenerateRes.pl MsgDef\TomboMsg_en.txt TomboMsg\en\TomboMsg.txt
%PERL% CheckGenerateRes.pl MsgDef\TomboMsg_jp.txt TomboMsg\jp\TomboMsg.txt
