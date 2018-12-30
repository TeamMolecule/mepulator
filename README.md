To get gdb to work, you need the following patch:

```
diff -u -r gdb-8.1/opcodes/mep-dis.c gdb-8.1-patched/opcodes/mep-dis.c
--- gdb-8.1/opcodes/mep-dis.c   2018-01-04 23:07:23.000000000 -0500
+++ gdb-8.1-patched/opcodes/mep-dis.c   2018-12-30 14:23:27.532487721 -0500
@@ -1380,7 +1380,7 @@
       int length;
       unsigned long insn_value_cropped;
 
-#ifdef CGEN_VALIDATE_INSN_SUPPORTED
+#if 0
       /* Not needed as insn shouldn't be in hash lists if not supported.  */
       /* Supported by this cpu?  */
       if (! mep_cgen_insn_supported (cd, insn))
```

Configure gdb with `./configure --enable-targets=mep`. Run mepulator with option `-g` and from gdb connect to port 7778.
