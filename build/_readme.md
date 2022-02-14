MWX ライブラリをビルドします。

Tools/Bash.cmd で窓を開いておいて、以下のように作業します。

```
$ cd ../mwx/build
$ make TWELITE=BLUE all deplopy
...
$ make TWELITE=RED all deplopy
...
```

ライブラリは `../../../lib` にコピーされます。