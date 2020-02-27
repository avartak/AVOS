
section .text

; void Context_Switch(struct Context** old_context, struct Context* new_context);

global KContext_Switch
KContext_Switch:
	mov  eax, [esp+4]
	mov  edx, [esp+8]
	mov  ecx, cr3	
	
	push ebp
	push ebx
	push esi
	push edi
	push ecx
	
	mov  [eax], esp
	mov  esp, edx
	
	pop  ecx
	pop  edi
	pop  esi
	pop  ebx
	pop  ebp
	
	mov  cr3, ecx

	ret

