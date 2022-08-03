use std::marker::PhantomData;

use std::ffi::IntoStringError;

use super::ffi;

pub struct Function<'a> {
    pub(crate) raw: ffi::BinaryenFunctionRef,
    _p: PhantomData<&'a super::Module>,
}

impl<'a> Function<'a> {
    pub fn body(&self) -> ffi::BinaryenExpressionRef {
        unsafe { ffi::BinaryenFunctionGetBody(self.raw) }
    }

    pub fn set_body(&self, body: ffi::BinaryenExpressionRef) {
        unsafe { ffi::BinaryenFunctionSetBody(self.raw, body) }
    }

    pub fn from_raw(raw: ffi::BinaryenFunctionRef) -> Function<'a> {
        Function {
            raw,
            _p: PhantomData,
        }
    }

    pub fn name(&self) -> Result<String, IntoStringError> {
        unsafe {
            let c_str = ffi::BinaryenFunctionGetName(self.raw);
            std::ffi::CStr::from_ptr(c_str).to_owned().into_string()
        }
    }
}
