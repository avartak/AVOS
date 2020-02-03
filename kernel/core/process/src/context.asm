
section .text

; void Context_Switch(struct Context** old_context, struct Context* new_context);

global Context_Switch
Context_Switch:
  mov  eax, [esp+4]
  mov  edx, [esp+8]

  push ebp
  push ebx
  push esi
  push edi

  mov  [eax], esp
  mov  esp, edx

  pop  edi
  pop  esi
  pop  ebx
  pop  ebp

  ret
