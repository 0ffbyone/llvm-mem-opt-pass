define dso_local i32 @foo(i32 noundef %x) {
entry:
  %call = tail call noalias dereferenceable_or_null(1024) ptr @my_malloc(i64 noundef 1024) #3
  %cmp = icmp eq i32 %x, 42
  br i1 %cmp, label %if.then, label %cleanup, !prof !1

if.then:
  tail call void @bar(ptr noundef %call) #4
  br label %cleanup

cleanup:
  %retval.0 = phi i32 [ 1, %if.then ], [ 0, %entry ]
  ret i32 %retval.0
}

declare noalias noundef ptr @my_malloc(i64 noundef) #1

declare void @bar(ptr noundef) local_unnamed_addr

attributes #1 = { allockind("alloc,uninitialized") }

!1 = !{!"branch_weights", i32 1, i32 2000}
