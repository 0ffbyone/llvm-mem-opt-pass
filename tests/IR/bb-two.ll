; ModuleID = 'bb-two.ll'


define ptr @foo(i32 %x, i32 %y) {
entry:
  %allocated = tail call noalias dereferenceable_or_null(1024) ptr @malloc(i64 noundef 1024) #3
  %bool_x = icmp eq i32 %x, 42
  %bool_y = icmp eq i32 %y, 42
  br i1 %bool_x, label %first, label %second

first:
  br i1 %bool_y, label %unlikely, label %first_likely, !prof !0

second:
  br i1 %bool_y, label %unlikely, label %second_likely, !prof !1

first_likely:
  ret ptr null

second_likely:
  br label %retlabel

unlikely:
  %new_ptr = tail call ptr @bar(ptr noundef %allocated) 
  br label %retlabel

retlabel:
  %0 = phi ptr [ %new_ptr, %unlikely ], [ null, %second_likely ]
  ret ptr %0
}

; Function Attrs: mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite)
declare noalias noundef ptr @malloc(i64 noundef) local_unnamed_addr #1


declare ptr @bar(ptr noundef) local_unnamed_addr #2

attributes #1 = { mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite) "alloc-family"="malloc" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }


!0 = !{!"branch_weights", i32 1, i32 2000}
!1 = !{!"branch_weights", i32 1, i32 2000}
