; ModuleID = './IR/main-test.ll'
source_filename = "./IR/main-test.ll"

define dso_local i32 @foo(i32 noundef %x) {
entry:
  %cmp = icmp eq i32 %x, 42
  br i1 %cmp, label %if.then, label %cleanup, !prof !0

if.then:                                          ; preds = %entry
  %call = tail call noalias dereferenceable_or_null(1024) ptr @my_malloc(i64 noundef 1024)
  tail call void @bar(ptr noundef %call)
  br label %cleanup

cleanup:                                          ; preds = %if.then, %entry
  %retval.0 = phi i32 [ 1, %if.then ], [ 0, %entry ]
  ret i32 %retval.0
}

; Function Attrs: allockind("alloc,uninitialized") allocsize(0)
declare noalias noundef ptr @my_malloc(i64 noundef) #0

declare void @bar(ptr noundef) local_unnamed_addr

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) }

!0 = !{!"branch_weights", i32 1, i32 2000}
